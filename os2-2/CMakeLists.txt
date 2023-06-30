cmake_minimum_required(VERSION 3.14)
project(os2_2 C)

set(CMAKE_C_STANDARD 11)

add_executable(os2_2 main.c buffer_pool.h producer.c consumer.c semaphores.h shared_memory.h)
