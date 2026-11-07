# linux-stream-relay-cpp

[linux-stream-relay](https://github.com/way-wei96/linux-stream-relay) 的 C++17 重写：Linux 用户态字节流中继。

## 当前进度

- `unique_fd.hpp`：`UniqueFd`
- `endpoint.hpp` / `.cpp`：`open_endpoint`
- `relay_io.hpp` / `.cpp`：`relay_bytes`（内部 `write_fully`）
- `main.cpp`：`parse_args` 小函数 + 组装

## 编译

```text
mkdir -p build
c++ -std=c++17 -Wall -Wextra -g -O0 -Isrc \
  -o build/relay src/main.cpp src/endpoint.cpp src/relay_io.cpp
```

（`compile_flags.txt` 已含 `-std=c++17`、`-Isrc`。）

## 运行

```text
echo 'hello' | ./build/relay
./build/relay some.bin
./build/relay some.bin out.bin
echo 'hello' | ./build/relay - out.bin
```
