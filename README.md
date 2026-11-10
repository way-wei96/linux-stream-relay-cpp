# linux-stream-relay-cpp

[linux-stream-relay](https://github.com/way-wei96/linux-stream-relay) 的 C++17 重写：Linux 用户态字节流中继。

## 当前进度

- `unique_fd.hpp`：`UniqueFd`（禁拷贝，可 move）
- `endpoint.hpp` / `.cpp`：`open_endpoint` → `optional<UniqueFd>`
- `relay_io.hpp` / `.cpp`：`write_fully`
- `ring_buffer.hpp`：`RingBuffer`（mutex + condition_variable；push 短超时可看见停）
- `stop_flag.hpp` / `.cpp`：`SIGINT` → 旗；读侧 `close` 优雅停，写侧排空
- `relay_workers.hpp` / `.cpp`：`reader_task` / `writer_task`（短超时 `poll` + `enum class WaitResult`）
- `main.cpp`：解析、打开；`unique_ptr` 握 `RingBuffer`；起线程、`join`
- `CMakeLists.txt`：C++17 目标 `relay`（`-Wall -Wextra`、pthread）

## 编译

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

产物：`build/relay`。也可用原来的手写 `c++ ...`（见 git 历史）；日常以 CMake 为准。  
（`compile_flags.txt` 仍给编辑器做补全用。）

## 运行

```text
echo 'hello' | ./build/relay
./build/relay some.bin
./build/relay some.bin out.bin
echo 'hello' | ./build/relay - out.bin
```
