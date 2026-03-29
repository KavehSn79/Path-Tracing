cmake_minimum_required(VERSION 3.10)
project(tinyobj)

add_library(tinyobj tiny_obj_loader.h tiny_obj_loader.cpp)
target_include_directories(tinyobj PUBLIC ${CMAKE_CURRENT_LIST_DIR})