#include "relay_io.hpp"

#include <cerrno>
#include <cstdio>
#include <unistd.h>

#include <array>
#include <cstdint>

namespace relay {
namespace {

bool write_fully(int out_fd, const std::uint8_t *buf, std::size_t nbytes)
{
  std::size_t off = 0;
  while (off < nbytes) {
    ssize_t w = ::write(out_fd, buf + off, nbytes - off);
    if (w < 0) {
      if (errno == EINTR)
        continue;
      return false;
    }
    off += static_cast<std::size_t>(w);
  }
  return true;
}

} // namespace

bool relay_bytes(int in_fd, int out_fd)
{
  std::array<std::uint8_t, 1024> buf{};

  for (;;) {
    ssize_t n = ::read(in_fd, buf.data(), buf.size());
    if (n < 0) {
      if (errno == EINTR)
        continue;
      std::perror("read");
      return false;
    }
    if (n == 0)
      return true;

    if (!write_fully(out_fd, buf.data(), static_cast<std::size_t>(n))) {
      std::perror("write");
      return false;
    }
  }
}

} // namespace relay
