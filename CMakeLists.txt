cmake_minimum_required(VERSION 3.16)
project(linux-stream-relay-cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(relay
  src/main.cpp
  src/endpoint.cpp
  src/relay_io.cpp
  src/relay_workers.cpp
  src/stop_flag.cpp
)

target_include_directories(relay PRIVATE src)
target_compile_options(relay PRIVATE -Wall -Wextra)
target_link_libraries(relay PRIVATE pthread)
