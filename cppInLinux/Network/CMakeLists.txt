cmake_minimum_required(VERSION 3.18.0)

project(Network)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

link_libraries(pthread)

add_executable(tcpServer.out src/tcpServer.cpp)
add_executable(multi_io.out src/multi_io.cpp)
add_executable(client.out src/client.cpp)