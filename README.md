# linux-stream-relay-cpp

[linux-stream-relay](https://github.com/way-wei96/linux-stream-relay) 的 C++17 重写：Linux 用户态字节流中继。

## 当前进度

`UniqueFd` → `unique_fd.hpp`；`open_endpoint` → `endpoint.hpp` / `endpoint.cpp`。

## 编译

必须 **`-std=c++17`**、**`-Isrc`**；多个 `.cpp` 一起编译链接。

```text
mkdir -p build
c++ -std=c++17 -Wall -Wextra -g -O0 -Isrc \
  -o build/relay src/main.cpp src/endpoint.cpp
```

（`compile_flags.txt` 已含这些选项，方便编辑器检查。）

## 运行

```text
echo 'hello' | ./build/relay
./build/relay some.bin
./build/relay some.bin out.bin
echo 'hello' | ./build/relay - out.bin
```
