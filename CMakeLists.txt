cmake_minimum_required(VERSION 2.8.9)
project(Mecacell)

set(CMAKE_CXX_FLAGS "-O3 -std=c++11 -Wall -Wextra -pedantic")
add_subdirectory(mecacell)
add_subdirectory(mecacellviewer)
