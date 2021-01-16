cmake_minimum_required(VERSION 3.16)
project(gtfo)

set(CMAKE_CXX_STANDARD 20)

#include_directories("~/nauty27r1")

add_executable(SteinerTripleSystems sts_toughness.cpp)
add_executable(IrregularToughness irregular_toughness.cpp)