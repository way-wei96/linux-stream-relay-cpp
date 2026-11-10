# linux-stream-relay-cpp

[linux-stream-relay](https://github.com/way-wei96/linux-stream-relay) 的 C++17 重写：Linux 用户态字节流中继。

## 当前进度

- `unique_fd.hpp`：`UniqueFd`（禁拷贝，可 move）
- `endpoint.hpp` / `.cpp`：`open_endpoint` → `optional<UniqueFd>`
- `relay_io.hpp` / `.cpp`：`write_fully`
- `ring_buffer.hpp`：`RingBuffer`（mutex + condition_variable；push 短超时可看见停）
- `stop_flag.hpp` / `.cpp`：`SIGINT` → 旗；读侧 `close` 优雅停，写侧排空
- `main.cpp`：读线程先短超时 `poll` 再 `read`/`push`；写线程 `pop`；主线程只 `join`

## 编译

```text
mkdir -p build
c++ -std=c++17 -Wall -Wextra -g -O0 -pthread -Isrc \
  -o build/relay src/main.cpp src/endpoint.cpp src/relay_io.cpp \
  src/stop_flag.cpp
```

（`compile_flags.txt` 已含常用选项；链接线程请加 `-pthread`。）

## 运行

```text
echo 'hello' | ./build/relay
./build/relay some.bin
./build/relay some.bin out.bin
echo 'hello' | ./build/relay - out.bin
```
