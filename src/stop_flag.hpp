#ifndef RELAY_STOP_FLAG_HPP
#define RELAY_STOP_FLAG_HPP

namespace relay {

/* 安装 SIGINT（Ctrl+C）。不加 SA_RESTART，好打断阻塞 read。 */
void install_stop_handlers();

bool stop_requested();

} // namespace relay

#endif
