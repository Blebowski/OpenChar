add_definitions(-DTEST_NETLIST_DIR="${CMAKE_CURRENT_SOURCE_DIR}/netlists")

macro(OC_BASIC_TEST test_name)
    add_executable(${test_name} ${test_name}.cpp)

    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/test/common)

    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src/Model)
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src/Simulation)
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src/Common)
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src/Tcl)
    target_include_directories(${test_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)

    add_test(NAME ${test_name} COMMAND ${test_name})
    target_link_libraries(${test_name} lib_open_char)
endmacro()

macro(OC_TCL_TEST test_name)
    add_test(NAME ${test_name} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/test_script.sh
                    ${CMAKE_BINARY_DIR}/src/open_char ${CMAKE_CURRENT_SOURCE_DIR}/${test_name})
endmacro()

add_subdirectory(model)
add_subdirectory(tcl)
add_subdirectory(algorithms)

