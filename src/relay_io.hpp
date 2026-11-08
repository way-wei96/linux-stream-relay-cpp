#ifndef RELAY_IO_HPP
#define RELAY_IO_HPP

#include <cstddef>
#include <cstdint>

namespace relay {

/* 把 data[0..nbytes) 写完；短写会继续。成功 true。 */
bool write_fully(int out_fd, const std::uint8_t *data, std::size_t nbytes);

} // namespace relay

#endif
