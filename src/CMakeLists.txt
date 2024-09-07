add_executable(boson
    boson/Main.cpp
    boson/ChatProvider.cpp
)

target_include_directories(boson PUBLIC ./)
target_link_libraries(boson dpp stackchat stackapi)


# vim:ft=cmake
