cmake_minimum_required(VERSION 3.10)
project(boson.cpp)

set (CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (DPP_BUILD_TEST OFF CACHE STRING "" FORCE)
set (DPP_INSTALL OFF CACHE STRING "" FORCE)
set (DPP_USE_EXTERNAL_JSON ON CACHE STRING "" FORCE)

include (FetchContent)
FetchContent_Declare(stackchat
    GIT_REPOSITORY https://github.com/LunarWatcher/stackchat.cpp)
FetchContent_Declare(stackapi
    GIT_REPOSITORY https://github.com/LunarWatcher/stackapi.cpp)
FetchContent_Declare(dpp
    GIT_REPOSITORY https://github.com/brainboxdotcc/DPP
    GIT_TAG "v10.0.30"
)

FetchContent_MakeAvailable(
    stackchat
    stackapi
    dpp
)

add_subdirectory(src)

add_custom_target(run
    COMMAND boson.cpp
    DEPENDS boson.cpp
    COMMENT "Run boson.cpp")
# vim:ft=cmake
