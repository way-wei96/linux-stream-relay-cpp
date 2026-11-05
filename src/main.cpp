#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <array>
#include <cstdint>

/* 打开输入：nullptr / "-" → stdin。失败 -1。 */
static int open_input(const char *path)
{
  if (path == nullptr || std::strcmp(path, "-") == 0)
    return STDIN_FILENO;

  int fd = ::open(path, O_RDONLY);
  if (fd < 0) {
    std::perror("open");
    return -1;
  }
  return fd;
}

static void close_if_opened(int fd)
{
  if (fd != STDIN_FILENO)
    ::close(fd);
}

/* 阻塞中继 in_fd → stdout。成功 true。 */
static bool relay_bytes(int in_fd)
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

    std::size_t off = 0;
    while (off < static_cast<std::size_t>(n)) {
      ssize_t w = ::write(STDOUT_FILENO, buf.data() + off, n - off);
      if (w < 0) {
        if (errno == EINTR)
          continue;
        std::perror("write");
        return false;
      }
      off += static_cast<std::size_t>(w);
    }
  }
}

/* 用法：relay [infile]；缺省 / "-" → stdin。输出固定 stdout。 */
int main(int argc, char **argv)
{
  const char *in_path = nullptr;

  if (argc > 2) {
    std::fprintf(stderr, "usage: %s [infile]\n", argv[0]);
    return 1;
  }
  if (argc == 2)
    in_path = argv[1];

  int in_fd = open_input(in_path);
  if (in_fd < 0)
    return 1;

  bool ok = relay_bytes(in_fd);
  close_if_opened(in_fd);
  return ok ? 0 : 1;
}
