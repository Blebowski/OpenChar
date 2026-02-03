cmake_minimum_required(VERSION 3.24.2)

project(characterizer)

include(CTest)

###############################################################################
# Settings
###############################################################################
set(CMAKE_CXX_STANDARD 20)


###############################################################################
# Dependencies
###############################################################################

# TODO: Check libraries are found!
find_package(TCL REQUIRED)

###############################################################################
# Build
###############################################################################
add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wno-format-security -O2)

add_subdirectory(src)
add_subdirectory(test)


