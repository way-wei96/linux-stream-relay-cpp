#ifndef RELAY_RING_BUFFER_HPP
#define RELAY_RING_BUFFER_HPP

#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace relay {

/*
 * 固定容量环形缓冲：读线程 push，写线程 pop。
 * close = 正常 EOF；fail = 任一侧出错，唤醒对方退出。
 */
class RingBuffer {
public:
  static constexpr std::size_t kCap = 4096;

  /* 阻塞把 src[0..n) 放进篮子。fail 后返回 false。 */
  bool push(const std::uint8_t *src, std::size_t n)
  {
    std::size_t off = 0;
    std::unique_lock<std::mutex> lk{mu_};
    while (off < n) {
      cv_.wait(lk, [this] { return size_ < kCap || failed_; });
      if (failed_)
        return false;
      while (off < n && size_ < kCap) {
        slots_[head_] = src[off++];
        head_ = (head_ + 1) % kCap;
        ++size_;
      }
      cv_.notify_all();
    }
    return true;
  }

  /* 从篮子舀到 dst，最多 n 字节。0 = 已 close/fail 且空。 */
  std::size_t pop(std::uint8_t *dst, std::size_t n)
  {
    std::unique_lock<std::mutex> lk{mu_};
    cv_.wait(lk, [this] { return size_ > 0 || closed_ || failed_; });
    if (size_ == 0)
      return 0;

    std::size_t got = 0;
    while (got < n && size_ > 0) {
      dst[got++] = slots_[tail_];
      tail_ = (tail_ + 1) % kCap;
      --size_;
    }
    cv_.notify_all();
    return got;
  }

  void close()
  {
    std::lock_guard<std::mutex> lk{mu_};
    closed_ = true;
    cv_.notify_all();
  }

  void fail()
  {
    std::lock_guard<std::mutex> lk{mu_};
    failed_ = true;
    closed_ = true;
    cv_.notify_all();
  }

private:
  std::uint8_t slots_[kCap]{};
  std::size_t head_ = 0;
  std::size_t tail_ = 0;
  std::size_t size_ = 0;
  bool closed_ = false;
  bool failed_ = false;
  std::mutex mu_;
  std::condition_variable cv_;
};

} // namespace relay

#endif
