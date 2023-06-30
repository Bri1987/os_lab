cmake_minimum_required(VERSION 3.14)
project(os2 C)

set(CMAKE_C_STANDARD 11)

add_executable(os2 main.c semaphores.c shared_memory.h shared_memory.c semaphores.h)
