#include "stop_flag.hpp"

#include <csignal>
#include <cstring>

namespace {

volatile std::sig_atomic_t g_stop;

void on_sigint(int)
{
  g_stop = 1;
}

} // namespace

namespace relay {

void install_stop_handlers()
{
  struct sigaction sa;
  std::memset(&sa, 0, sizeof sa);
  sa.sa_handler = on_sigint;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGINT, &sa, nullptr);
}

bool stop_requested()
{
  return g_stop != 0;
}

} // namespace relay
