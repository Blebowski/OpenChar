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

add_executable(open_char)

target_compile_options(open_char PRIVATE -Wall -Wextra -Wpedantic -Wconversion
                                         -Wno-format-security -O2)

target_include_directories(open_char PRIVATE src)
target_include_directories(open_char PRIVATE ${TCL_INCLUDE_PATH})

add_subdirectory(src)
add_subdirectory(test)

target_link_libraries(open_char ${TCL_LIBRARY})
