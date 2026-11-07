#ifndef RELAY_UNIQUE_FD_HPP
#define RELAY_UNIQUE_FD_HPP

#include <unistd.h>

namespace relay {

/*
 * 只管一件事：对象没了且 owns 为 true 时 close。
 * 禁止拷贝：两个对象握同一 fd 会 close 两次。
 */
class UniqueFd {
public:
  UniqueFd(int fd, bool owns) : fd_(fd), owns_(owns) {}

  ~UniqueFd()
  {
    if (owns_ && fd_ >= 0)
      ::close(fd_);
  }

  UniqueFd(const UniqueFd &) = delete;
  UniqueFd &operator=(const UniqueFd &) = delete;

  int get() const { return fd_; }

private:
  int fd_;
  bool owns_;
};

} // namespace relay

#endif
