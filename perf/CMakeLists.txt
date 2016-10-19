add_definitions(-std=gnu11)
add_definitions(-g)
add_definitions(-O3)

macro (add_perf_exec NAME)
    add_executable (${NAME} ${NAME}.c)
    target_link_libraries (${NAME} ${PROJECT_NAME})
    target_include_directories (${NAME} PUBLIC ../src)
endmacro (add_perf_exec)

add_perf_exec(parser_perf)
