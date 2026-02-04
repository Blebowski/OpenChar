add_library(lib_open_char STATIC)

target_include_directories(lib_open_char PUBLIC Model)
target_include_directories(lib_open_char PUBLIC Simulation)
target_include_directories(lib_open_char PUBLIC Tcl)
target_include_directories(lib_open_char PUBLIC Common)
target_include_directories(lib_open_char PUBLIC .)
target_include_directories(lib_open_char PUBLIC ${TCL_INCLUDE_PATH})

add_subdirectory(Model)
add_subdirectory(Simulation)
add_subdirectory(Tcl)

add_executable(open_char open_char.cpp)
set_target_properties(open_char PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(open_char PUBLIC Model)
target_include_directories(open_char PUBLIC Simulation)
target_include_directories(open_char PUBLIC Tcl)
target_include_directories(open_char PUBLIC Common)
target_include_directories(open_char PUBLIC .)
target_include_directories(open_char PUBLIC ${TCL_INCLUDE_PATH})

target_link_libraries(open_char PRIVATE lib_open_char)
target_link_libraries(open_char PRIVATE ${TCL_LIBRARY} ${TCLREADLINE_LIB} ${READLINE_LIB})
