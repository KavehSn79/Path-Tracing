cmake_minimum_required(VERSION 3.10)
project(json)

add_library(json INTERFACE) # Adding json.hpp here requires CMake > 3.19
target_include_directories(json INTERFACE ${CMAKE_CURRENT_LIST_DIR})