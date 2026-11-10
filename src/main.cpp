#include "endpoint.hpp"
#include "relay_io.hpp"
#include "ring_buffer.hpp"
#include "stop_flag.hpp"
#include "unique_fd.hpp"

#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <thread>

static constexpr int kReaderPollMs = 100;

/* 解析 [infile [outfile]]。成功 true。 */
static bool parse_args(int argc, char **argv,
                       std::optional<std::string> *in_path,
                       std::optional<std::string> *out_path)
{
  in_path->reset();
  out_path->reset();

  if (argc > 3) {
    std::fprintf(stderr, "usage: %s [infile [outfile]]\n", argv[0]);
    return false;
  }
  if (argc >= 2)
    *in_path = argv[1];
  if (argc == 3)
    *out_path = argv[2];
  return true;
}

/*
 * 短超时 poll，等 in_fd 可读。
 * 1 = 可读/对端挂起，可去 read；0 = 该停；-1 = 失败。
 */
static int wait_input_readable(int in_fd)
{
  for (;;) {
    if (relay::stop_requested())
      return 0;

    pollfd pfd{};
    pfd.fd = in_fd;
    pfd.events = POLLIN;

    int rc = ::poll(&pfd, 1, kReaderPollMs);
    if (rc < 0) {
      if (errno == EINTR) {
        if (relay::stop_requested())
          return 0;
        continue;
      }
      return -1;
    }
    if (rc == 0)
      continue;

    if (pfd.revents & (POLLERR | POLLNVAL))
      return -1;
    if (pfd.revents & (POLLIN | POLLHUP))
      return 1;
  }
}

/* 只读：先 poll 再 read。EOF / Ctrl+C → close；出错 fail。 */
static void reader_task(int in_fd, relay::RingBuffer *ring, bool *reader_ok)
{
  std::array<std::uint8_t, 1024> chunk{};
  for (;;) {
    if (relay::stop_requested()) {
      ring->close();
      return;
    }

    int wait_status = wait_input_readable(in_fd);
    if (wait_status == 0) {
      ring->close();
      return;
    }
    if (wait_status < 0) {
      std::perror("poll");
      *reader_ok = false;
      ring->fail();
      return;
    }

    ssize_t n = ::read(in_fd, chunk.data(), chunk.size());
    if (n < 0) {
      if (errno == EINTR) {
        if (relay::stop_requested()) {
          ring->close();
          return;
        }
        continue;
      }
      std::perror("read");
      *reader_ok = false;
      ring->fail();
      return;
    }
    if (n == 0) {
      ring->close();
      return;
    }
    if (!ring->push(chunk.data(), static_cast<std::size_t>(n))) {
      if (relay::stop_requested()) {
        ring->close();
        return;
      }
      *reader_ok = false;
      return;
    }
  }
}

/* 只写：ring → fd。pop 到 0 结束；写出错 fail。优雅停仍排空。 */
static void writer_task(int out_fd, relay::RingBuffer *ring, bool *writer_ok)
{
  std::array<std::uint8_t, 1024> chunk{};
  for (;;) {
    std::size_t n = ring->pop(chunk.data(), chunk.size());
    if (n == 0)
      return;
    if (!relay::write_fully(out_fd, chunk.data(), n)) {
      std::perror("write");
      *writer_ok = false;
      ring->fail();
      return;
    }
  }
}

int main(int argc, char **argv)
{
  relay::install_stop_handlers();

  std::optional<std::string> in_path;
  std::optional<std::string> out_path;

  if (!parse_args(argc, argv, &in_path, &out_path))
    return 1;

  std::optional<relay::UniqueFd> input =
      relay::open_endpoint(in_path, STDIN_FILENO, O_RDONLY);
  if (!input)
    return 1;

  std::optional<relay::UniqueFd> output = relay::open_endpoint(
      out_path, STDOUT_FILENO, O_WRONLY | O_CREAT | O_TRUNC);
  if (!output)
    return 1;

  relay::RingBuffer ring;
  bool reader_ok = true;
  bool writer_ok = true;
  std::thread reader(reader_task, input->get(), &ring, &reader_ok);
  std::thread writer(writer_task, output->get(), &ring, &writer_ok);
  reader.join();
  writer.join();
  return (reader_ok && writer_ok) ? 0 : 1;
}
