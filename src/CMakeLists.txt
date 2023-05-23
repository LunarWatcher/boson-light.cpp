add_executable(boson
    boson/Main.cpp
)

target_include_directories(boson PUBLIC ./)
target_link_libraries(boson stackchat stackapi)


# vim:ft=cmake
