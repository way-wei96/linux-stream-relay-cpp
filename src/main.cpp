#include <cerrno>
#include <cstdio>
#include <unistd.h>

#include <array>
#include <cstdint>

/* 第一刀：stdin → stdout 阻塞字节中继。 */
int main()
{
  std::array<std::uint8_t, 1024> buf{};

  for (;;) {
    ssize_t n = ::read(STDIN_FILENO, buf.data(), buf.size());
    if (n < 0) {
      if (errno == EINTR)
        continue;
      std::perror("read");
      return 1;
    }
    if (n == 0)
      return 0;

    std::size_t off = 0;
    while (off < static_cast<std::size_t>(n)) {
      ssize_t w = ::write(STDOUT_FILENO, buf.data() + off, n - off);
      if (w < 0) {
        if (errno == EINTR)
          continue;
        std::perror("write");
        return 1;
      }
      off += static_cast<std::size_t>(w);
    }
  }
}
