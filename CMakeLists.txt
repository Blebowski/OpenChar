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

find_package(TCL REQUIRED)
find_library(READLINE_LIB NAMES readline REQUIRED)
find_library(TCLREADLINE_LIB NAMES tclreadline REQUIRED)

###############################################################################
# Build
###############################################################################
add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wno-format-security -O2 -g)
add_link_options(-rdynamic)

add_subdirectory(doc)
add_subdirectory(src)
add_subdirectory(test)


