#include "relay_workers.hpp"

#include "relay_io.hpp"
#include "stop_flag.hpp"

#include <cerrno>
#include <cstdio>
#include <poll.h>
#include <unistd.h>

#include <array>
#include <cstdint>

namespace relay {
namespace {

constexpr int kReaderPollMs = 100;

enum class WaitResult {
  Ready, /* 可读或对端挂起，可去 read */
  Stop,  /* 停旗，该收工 */
  Error, /* poll 失败 */
};

/* 短超时 poll，等 in_fd 可读。 */
WaitResult wait_input_readable(int in_fd)
{
  for (;;) {
    if (stop_requested())
      return WaitResult::Stop;

    pollfd pfd{};
    pfd.fd = in_fd;
    pfd.events = POLLIN;

    int rc = ::poll(&pfd, 1, kReaderPollMs);
    if (rc < 0) {
      if (errno == EINTR) {
        if (stop_requested())
          return WaitResult::Stop;
        continue;
      }
      return WaitResult::Error;
    }
    if (rc == 0)
      continue;

    if (pfd.revents & (POLLERR | POLLNVAL))
      return WaitResult::Error;
    if (pfd.revents & (POLLIN | POLLHUP))
      return WaitResult::Ready;
  }
}

} // namespace

void reader_task(int in_fd, RingBuffer *ring, bool *reader_ok)
{
  std::array<std::uint8_t, 1024> chunk{};
  for (;;) {
    if (stop_requested()) {
      ring->close();
      return;
    }

    WaitResult wait_status = wait_input_readable(in_fd);
    if (wait_status == WaitResult::Stop) {
      ring->close();
      return;
    }
    if (wait_status == WaitResult::Error) {
      std::perror("poll");
      *reader_ok = false;
      ring->fail();
      return;
    }

    ssize_t n = ::read(in_fd, chunk.data(), chunk.size());
    if (n < 0) {
      if (errno == EINTR) {
        if (stop_requested()) {
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
      if (stop_requested()) {
        ring->close();
        return;
      }
      *reader_ok = false;
      return;
    }
  }
}

void writer_task(int out_fd, RingBuffer *ring, bool *writer_ok)
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

} // namespace relay
