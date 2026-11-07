# linux-stream-relay-cpp

[linux-stream-relay](https://github.com/way-wei96/linux-stream-relay) 的 C++17 重写：Linux 用户态字节流中继。

## 当前进度

单文件：`namespace relay`；极简 `UniqueFd`（析构 close；拷贝已 `= delete`）。

## 编译

必须带上 **`-std=c++17`**（`std::optional` 需要）；漏掉会报 `no template named 'optional'`。

```text
mkdir -p build
c++ -std=c++17 -Wall -Wextra -g -O0 -o build/relay src/main.cpp
```

（仓库里有 `compile_flags.txt`，方便编辑器按 C++17 做检查。）

## 运行

```text
echo 'hello' | ./build/relay
./build/relay some.bin
./build/relay some.bin out.bin
echo 'hello' | ./build/relay - out.bin
```
