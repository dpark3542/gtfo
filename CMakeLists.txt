cmake_minimum_required(VERSION 3.16)
project(gtfo)

set(CMAKE_CXX_STANDARD 20)

add_executable(SteinerTripleSystems sts_toughness.cpp)
add_executable(Hamming hamming.cpp)

add_executable(test test.cpp)