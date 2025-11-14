cmake_minimum_required(VERSION 3.15)
project(idlib_standalone)

file(GLOB_RECURSE idlib_sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
list(REMOVE_ITEM idlib_sources "test.cpp")

add_library(idlib_standalone STATIC ${idlib_sources})
add_library(idlib_standalone_shared SHARED ${idlib_sources})

target_include_directories(idlib_standalone PUBLIC .)
target_include_directories(idlib_standalone_shared PUBLIC .)
