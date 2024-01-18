cmake_minimum_required(VERSION 3.27)
project(cache_lab C)

set(CMAKE_C_STANDARD 11)



include_directories(.)
add_executable(csim cachelab.c  csim.c)