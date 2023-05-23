cmake_minimum_required(VERSION 3.10)
project(boson.cpp)

set (CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include (FetchContent)
FetchContent_Declare(stackchat
    GIT_REPOSITORY https://github.com/LunarWatcher/stackchat.cpp)
FetchContent_Declare(stackapi
    GIT_REPOSITORY https://github.com/LunarWatcher/stackapi.cpp)
FetchContent_MakeAvailable(stackchat)
FetchContent_MakeAvailable(stackapi)

add_subdirectory(src)

add_custom_target(run
    COMMAND boson.cpp
    DEPENDS boson.cpp
    COMMENT "Run boson.cpp")
# vim:ft=cmake
