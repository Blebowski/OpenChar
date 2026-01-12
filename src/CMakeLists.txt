

target_include_directories(open_char PRIVATE Model)
target_include_directories(open_char PRIVATE Simulation)

add_subdirectory(Model)
add_subdirectory(Simulation)

target_sources(open_char PRIVATE
    open_char.cpp
)