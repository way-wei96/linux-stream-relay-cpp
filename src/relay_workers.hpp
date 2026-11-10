#ifndef RELAY_WORKERS_HPP
#define RELAY_WORKERS_HPP

#include "ring_buffer.hpp"

namespace relay {

/* 只读：先 poll 再 read → push。EOF / Ctrl+C → close；出错 fail。 */
void reader_task(int in_fd, RingBuffer *ring, bool *reader_ok);

/* 只写：pop → write。优雅停仍排空。 */
void writer_task(int out_fd, RingBuffer *ring, bool *writer_ok);

} // namespace relay

#endif
