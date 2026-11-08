#include "endpoint.hpp"
#include "relay_io.hpp"
#include "unique_fd.hpp"

#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

#include <optional>
#include <string>
#include <thread>

/* 解析 [infile [outfile]]。成功 true。 */
static bool parse_args(int argc, char **argv,
                       std::optional<std::string> *in_path,
                       std::optional<std::string> *out_path)
{
  in_path->reset();
  out_path->reset();

  if (argc > 3) {
    std::fprintf(stderr, "usage: %s [infile [outfile]]\n", argv[0]);
    return false;
  }
  if (argc >= 2)
    *in_path = argv[1];
  if (argc == 3)
    *out_path = argv[2];
  return true;
}

/* 本半步：读线程里做完整中继。写线程仍空跑，下一刀再拆读写。 */
static void reader_task(int in_fd, int out_fd, bool *ok)
{
  *ok = relay::relay_bytes(in_fd, out_fd);
}

static void writer_task() {}

int main(int argc, char **argv)
{
  std::optional<std::string> in_path;
  std::optional<std::string> out_path;

  if (!parse_args(argc, argv, &in_path, &out_path))
    return 1;

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

  bool ok = false;
  std::thread reader(reader_task, in_fd.get(), out_fd.get(), &ok);
  std::thread writer(writer_task);
  reader.join();
  writer.join();
  return ok ? 0 : 1;
}
