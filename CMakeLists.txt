cmake_minimum_required(VERSION 3.10)
project(papagaio C)

set(CMAKE_C_STANDARD 99)

include_directories(.)

# Main library (static)
add_library(libpapagaio STATIC
  src/papagaio.c
  lib/libregexp/libregexp.c
  lib/libregexp/cutils.c
  lib/libregexp/libunicode.c
)
if (NOT MSVC)
  target_link_libraries(libpapagaio m)
endif()

# Main executable
add_executable(papagaio src/main.c)
if (NOT MSVC)
  target_link_libraries(papagaio libpapagaio m)
else()
  target_link_libraries(papagaio libpapagaio)
endif()

# Installation
install(TARGETS papagaio DESTINATION bin)
install(FILES src/papagaio.h DESTINATION include)
install(TARGETS libpapagaio DESTINATION lib)
