#include "endpoint.hpp"
#include "ring_buffer.hpp"
#include "unique_fd.hpp"

#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <thread>

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

static bool write_fully(int out_fd, const std::uint8_t *data, std::size_t nbytes)
{
  std::size_t off = 0;
  while (off < nbytes) {
    ssize_t w = ::write(out_fd, data + off, nbytes - off);
    if (w < 0) {
      if (errno == EINTR)
        continue;
      return false;
    }
    off += static_cast<std::size_t>(w);
  }
  return true;
}

/* 只读：fd → ring。EOF 后 close；出错 fail。 */
static void reader_task(int in_fd, relay::RingBuffer *ring, bool *reader_ok)
{
  std::array<std::uint8_t, 1024> chunk{};
  for (;;) {
    ssize_t n = ::read(in_fd, chunk.data(), chunk.size());
    if (n < 0) {
      if (errno == EINTR)
        continue;
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
      *reader_ok = false;
      return;
    }
  }
}

/* 只写：ring → fd。pop 到 0 结束；写出错 fail。 */
static void writer_task(int out_fd, relay::RingBuffer *ring, bool *writer_ok)
{
  std::array<std::uint8_t, 1024> chunk{};
  for (;;) {
    std::size_t n = ring->pop(chunk.data(), chunk.size());
    if (n == 0)
      return;
    if (!write_fully(out_fd, chunk.data(), n)) {
      std::perror("write");
      *writer_ok = false;
      ring->fail();
      return;
    }
  }
}

int main(int argc, char **argv)
{
  std::optional<std::string> in_path;
  std::optional<std::string> out_path;

  if (!parse_args(argc, argv, &in_path, &out_path))
    return 1;

  int in_raw = -1;
  bool in_owns = false;
  if (!relay::open_endpoint(in_path, STDIN_FILENO, O_RDONLY, &in_raw,
                            &in_owns))
    return 1;
  relay::UniqueFd in_fd(in_raw, in_owns);

  int out_raw = -1;
  bool out_owns = false;
  if (!relay::open_endpoint(out_path, STDOUT_FILENO,
                            O_WRONLY | O_CREAT | O_TRUNC, &out_raw,
                            &out_owns))
    return 1;
  relay::UniqueFd out_fd(out_raw, out_owns);

  relay::RingBuffer ring;
  bool reader_ok = true;
  bool writer_ok = true;
  std::thread reader(reader_task, in_fd.get(), &ring, &reader_ok);
  std::thread writer(writer_task, out_fd.get(), &ring, &writer_ok);
  reader.join();
  writer.join();
  return (reader_ok && writer_ok) ? 0 : 1;
}
