#
#   Copyright (c) 2012 Martin Sustrik  All rights reserved.
#   Copyright (c) 2013 GoPivotal, Inc.  All rights reserved.
#   Copyright (c) 2015-2016 Jack R. Dunaway. All rights reserved.
#   Copyright 2016 Garrett D'Amore <garrett@damore.org>
#   Copyright 2016 Franklin "Snaipe" Mathieu <franklinmathieu@gmail.com>
#
#   Permission is hereby granted, free of charge, to any person obtaining a copy
#   of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom
#   the Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
#   IN THE SOFTWARE.
#

set (TEST_HELPERS_SRC
  ../deps/console-colors/console-colors.c
  ../deps/describe/describe.c
)

add_library (${PROJECT_NAME}-test-helpers STATIC ${TEST_HELPERS_SRC})
target_include_directories (${PROJECT_NAME}-test-helpers PUBLIC ../deps)

enable_testing ()
set (all_tests "")

macro (framer_add_test NAME)
    list (APPEND all_tests ${NAME})
    add_executable (${NAME} ${NAME}.c)
    target_link_libraries (${NAME} ${PROJECT_NAME} ${PROJECT_NAME}-test-helpers)
    target_include_directories (${NAME} PUBLIC ../deps)
    target_include_directories (${NAME} PUBLIC ../src)
    add_test (NAME ${NAME} COMMAND ${NAME})
    # set_tests_properties (${NAME} PROPERTIES TIMEOUT ${TIMEOUT})
endmacro (framer_add_test)

framer_add_test (parser)
framer_add_test (frame-list)

set (runner_script ${PROJECT_SOURCE_DIR}/tests/run_all.sh)
set (runner_script_local ./run_all.sh)

configure_file(${runner_script} ${runner_script_local} COPYONLY)

add_definitions(-std=gnu11)
add_definitions(-g)
