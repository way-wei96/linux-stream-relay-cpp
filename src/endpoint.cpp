#include "endpoint.hpp"

#include <cstdio>
#include <fcntl.h>

namespace relay {

bool open_endpoint(const std::optional<std::string> &path, int std_fd, int flags,
                   int *out_fd, bool *out_owns)
{
  if (!path.has_value() || *path == "-") {
    *out_fd = std_fd;
    *out_owns = false;
    return true;
  }

  int fd = (flags & O_CREAT) != 0 ? ::open(path->c_str(), flags, 0666)
                                  : ::open(path->c_str(), flags);
  if (fd < 0) {
    std::perror("open");
    return false;
  }
  *out_fd = fd;
  *out_owns = true;
  return true;
}

} // namespace relay
