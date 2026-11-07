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
 * 打开路径；无值 / "-" → 返回 std_fd。
 * 带 O_CREAT 时用 mode 0666。失败 -1。
 */
int open_endpoint(const std::optional<std::string> &path, int std_fd, int flags)
{
  if (!path.has_value() || *path == "-")
    return std_fd;

  int fd = (flags & O_CREAT) != 0 ? ::open(path->c_str(), flags, 0666)
                                  : ::open(path->c_str(), flags);
  if (fd < 0) {
    std::perror("open");
    return -1;
  }
  return fd;
}

void close_if_opened(int fd, int std_fd)
{
  if (fd != std_fd)
    ::close(fd);
}

/* 把 buf 里 nbytes 全部写出；短写则继续。成功 true。 */
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

/* 阻塞中继 in_fd → out_fd。成功 true。 */
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

/* 用法：relay [infile [outfile]]；缺省 / "-" → 标准流。 */
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

  int in_fd = relay::open_endpoint(in_path, STDIN_FILENO, O_RDONLY);
  if (in_fd < 0)
    return 1;

  int out_fd = relay::open_endpoint(out_path, STDOUT_FILENO,
                                    O_WRONLY | O_CREAT | O_TRUNC);
  if (out_fd < 0) {
    relay::close_if_opened(in_fd, STDIN_FILENO);
    return 1;
  }

  bool ok = relay::relay_bytes(in_fd, out_fd);
  relay::close_if_opened(in_fd, STDIN_FILENO);
  relay::close_if_opened(out_fd, STDOUT_FILENO);
  return ok ? 0 : 1;
}
