cmake_minimum_required(VERSION 3.16)
project(gtfo)

set(CMAKE_CXX_STANDARD 20)

add_executable(extendable extendable.cpp)
target_include_directories(extendable PUBLIC /home/dpark/nauty27r3)
add_library(nauty STATIC
    /home/dpark/nauty27r3/nauty.c
    /home/dpark/nauty27r3/nautil.c
    /home/dpark/nauty27r3/naugraph.c
    /home/dpark/nauty27r3/schreier.c
    /home/dpark/nauty27r3/naurng.c
)
target_link_libraries(extendable nauty)

add_executable(test test.cpp)