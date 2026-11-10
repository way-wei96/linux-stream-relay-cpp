#include "endpoint.hpp"
#include "relay_workers.hpp"
#include "ring_buffer.hpp"
#include "stop_flag.hpp"
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

int main(int argc, char **argv)
{
  relay::install_stop_handlers();

  std::optional<std::string> in_path;
  std::optional<std::string> out_path;

  if (!parse_args(argc, argv, &in_path, &out_path))
    return 1;

  std::optional<relay::UniqueFd> input =
      relay::open_endpoint(in_path, STDIN_FILENO, O_RDONLY);
  if (!input)
    return 1;

  std::optional<relay::UniqueFd> output = relay::open_endpoint(
      out_path, STDOUT_FILENO, O_WRONLY | O_CREAT | O_TRUNC);
  if (!output)
    return 1;

  relay::RingBuffer ring;
  bool reader_ok = true;
  bool writer_ok = true;
  std::thread reader(relay::reader_task, input->get(), &ring, &reader_ok);
  std::thread writer(relay::writer_task, output->get(), &ring, &writer_ok);
  reader.join();
  writer.join();
  return (reader_ok && writer_ok) ? 0 : 1;
}
