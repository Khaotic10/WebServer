cmake_minimum_required(VERSION 3.16)
project(TCP_server)

set(CMAKE_CXX_STANDARD 20)

add_executable(TCP_server server_m.cpp src/server/server.cpp)

add_executable(TCP_client client_m.cpp src/client/client.cpp src/server/server.cpp src/cgi/test.cpp src/cgi/test.h)
