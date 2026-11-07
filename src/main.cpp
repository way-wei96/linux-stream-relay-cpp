#include "endpoint.hpp"
#include "relay_io.hpp"
#include "unique_fd.hpp"

#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

#include <optional>
#include <string>

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
