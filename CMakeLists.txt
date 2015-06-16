cmake_minimum_required(VERSION 2.8)
project(MecaCell)

#set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-O3 -std=c++11 -Wall -Wextra -pedantic")
message(${CMAKE_CXX_COMPILER})
add_subdirectory(MecaCell)
add_subdirectory(MecaCellViewer)
add_subdirectory(Examples)
