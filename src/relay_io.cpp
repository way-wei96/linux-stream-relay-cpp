#include "relay_io.hpp"

#include <cerrno>
#include <unistd.h>

namespace relay {

bool write_fully(int out_fd, const std::uint8_t *data, std::size_t nbytes)
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

} // namespace relay
