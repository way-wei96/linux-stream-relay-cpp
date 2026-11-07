#ifndef RELAY_IO_HPP
#define RELAY_IO_HPP

namespace relay {

/* 阻塞中继 in_fd → out_fd。成功 true。 */
bool relay_bytes(int in_fd, int out_fd);

} // namespace relay

#endif
