#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

#include <array>
#include <cstdint>
#include <optional>
#include <string>

namespace relay {

/*
 * 只管一件事：对象没了且 owns 为 true 时 close。
 * 不 return / 不拷贝这个对象，避免双 close；所以不必写 delete、移动。
 */
class UniqueFd {
public:
  UniqueFd(int fd, bool owns) : fd_(fd), owns_(owns) {}

  ~UniqueFd()
  {
    if (owns_ && fd_ >= 0)
      ::close(fd_);
  }

  int get() const { return fd_; }

private:
  int fd_;
  bool owns_;
};

/*
 * 打开路径；无值 / "-" → *out_fd = std_fd，*out_owns = false。
 * 成功 true；失败 false。
 */
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

int main(int argc, char **argv)
{
  std::optional<std::string> in_path;
  std::optional<std::string> out_path;

  if (argc > 3) {
    std::fprintf(stderr, "usage: %s [infile [outfile]]\n", argv[0]);
    return 1;
  }
  if (argc >= 2)
    in_path = argv[1];
  if (argc == 3)
    out_path = argv[2];

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

  return relay::relay_bytes(in_fd.get(), out_fd.get()) ? 0 : 1;
}
