
add_executable(doc_generator)

target_include_directories(doc_generator PUBLIC ../src/Tcl)
target_include_directories(doc_generator PUBLIC ../src/Common)

target_sources(doc_generator PRIVATE
    doc_generator.cpp
)

add_custom_target(generate_commands_and_variables ALL
                  DEPENDS doc_generator
                  COMMAND ./doc_generator ${CMAKE_SOURCE_DIR}/doc/manual)

target_link_libraries(doc_generator PRIVATE lib_open_char)
target_link_libraries(doc_generator PRIVATE ${TCL_LIBRARY} ${TCLREADLINE_LIB} ${READLINE_LIB})