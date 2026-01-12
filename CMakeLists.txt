cmake_minimum_required(VERSION 3.24.2)

project(characterizer)

add_executable(open_char)

target_include_directories(open_char PRIVATE src)

add_subdirectory(src)
