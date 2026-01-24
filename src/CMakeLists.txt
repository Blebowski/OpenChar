

target_include_directories(open_char PRIVATE Model)
target_include_directories(open_char PRIVATE Simulation)
target_include_directories(open_char PRIVATE Tcl)

add_subdirectory(Model)
add_subdirectory(Simulation)
add_subdirectory(Tcl)

target_sources(open_char PRIVATE
    open_char.cpp
)