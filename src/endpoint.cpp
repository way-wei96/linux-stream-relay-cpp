#include "endpoint.hpp"

#include <cstdio>
#include <fcntl.h>

namespace relay {

std::optional<UniqueFd> open_endpoint(const std::optional<std::string> &path,
                                      int std_fd, int flags)
{
  if (!path.has_value() || *path == "-")
    return UniqueFd{std_fd, false};

  int fd = (flags & O_CREAT) != 0 ? ::open(path->c_str(), flags, 0666)
                                  : ::open(path->c_str(), flags);
  if (fd < 0) {
    std::perror("open");
    return std::nullopt;
  }
  return UniqueFd{fd, true};
}

} // namespace relay
