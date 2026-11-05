# linux-stream-relay-cpp

[linux-stream-relay](https://github.com/way-wei96/linux-stream-relay) 的 C++17 重写：Linux 用户态字节流中继。

## 当前进度

第一刀：单文件 `main.cpp`，stdin → stdout 阻塞中继。

## 编译

```text
mkdir -p build
c++ -std=c++17 -Wall -Wextra -g -O0 -o build/relay src/main.cpp
```

## 运行

```text
echo 'hello' | ./build/relay
```
