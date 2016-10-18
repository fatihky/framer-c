set (LIBFRAMER_SRC
  frame.c
  parser.c
  cbuf.c
  frame-list.c
  err.c
)

add_library (${PROJECT_NAME} STATIC ${LIBFRAMER_SRC})

add_definitions(-std=gnu11)
add_definitions(-g)
