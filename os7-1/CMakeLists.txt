cmake_minimum_required(VERSION 3.14)
project(os7_1 C)

set(CMAKE_C_STANDARD 11)

include_directories( /usr/src/kernels/3.10.0-1160.71.1.el7.x86_64/include)
add_executable(os7_1 main.c)
