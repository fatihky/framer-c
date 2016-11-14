set (LIBFRAMER_SRC
  frame.c
  parser.c
  cbuf.c
  frame-list.c
  err.c
)

set (LIBFRAMER_HEADERS
  attr.h
  cbuf.h
  cont.h
  err.h
  fast.h
  frame.h
  frame-list.h
  parser.h
  util.h
)

add_library (${PROJECT_NAME} STATIC ${LIBFRAMER_SRC})

add_definitions(-std=gnu11)
add_definitions(-g)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES ${LIBFRAMER_HEADERS} DESTINATION include/framer)
