cmake_minimum_required(VERSION 3.24.2)

project(characterizer)

###############################################################################
# Dependencies
###############################################################################

# TODO: Check libraries are found!
find_package(TCL REQUIRED)

###############################################################################
# Build
###############################################################################

add_executable(open_char)

target_compile_options(open_char PRIVATE -Wall -Wextra -Wpedantic -Wconversion)

target_include_directories(open_char PRIVATE src)
target_include_directories(open_char PRIVATE ${TCL_INCLUDE_PATH})

add_subdirectory(thirdparty/fmt)
add_subdirectory(src)

target_link_libraries(open_char fmt::fmt)
target_link_libraries(open_char ${TCL_LIBRARY})
