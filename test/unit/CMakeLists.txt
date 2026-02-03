add_executable(ExtractLogicTableTest ExtractLogicTableTest.cpp)

target_include_directories(ExtractLogicTableTest PRIVATE ${CMAKE_SOURCE_DIR}/src/Model)
target_include_directories(ExtractLogicTableTest PRIVATE ${CMAKE_SOURCE_DIR}/src/Simulation)
target_include_directories(ExtractLogicTableTest PRIVATE ${CMAKE_SOURCE_DIR}/src/Common)
target_include_directories(ExtractLogicTableTest PRIVATE ${CMAKE_SOURCE_DIR}/src/Tcl)
target_include_directories(ExtractLogicTableTest PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_test(NAME ExtractLogicTableTest COMMAND ExtractLogicTableTest)
target_link_libraries(ExtractLogicTableTest lib_open_char)
