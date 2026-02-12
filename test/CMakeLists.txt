add_definitions(-DTEST_COMMON_DIR="${CMAKE_CURRENT_SOURCE_DIR}/common")

macro(OPEN_CHAR_TEST test_name)
    add_executable(${test_name} ${test_name}.cpp)

    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src/Model)
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src/Simulation)
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src/Common)
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src/Tcl)
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)

    add_test(NAME ${test_name} COMMAND ${test_name})
    target_link_libraries(${test_name} lib_open_char)
endmacro()

add_subdirectory(unit)
add_subdirectory(algorithms)
