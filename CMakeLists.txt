cmake_minimum_required(VERSION 3.10)
project(papagaio C)

set(CMAKE_C_STANDARD 99)

include_directories(. libs/quickjs libs/wasm3/source)

file(GLOB WASM3_SRC "libs/wasm3/source/*.c")

# Main library (static)
# Determine if we should include quickjs-libc.c (requires unistd.h, etc)
if(MSVC)
  set(QUICKJS_LIBC "")
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
  # Force NaN boxing on 64-bit MSVC
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions(-DJS_NAN_BOXING)
  endif()
else()
  set(QUICKJS_LIBC "libs/quickjs/quickjs-libc.c")
endif()

add_library(libpapagaio STATIC
  src/papagaio.c
  libs/quickjs/quickjs.c
  libs/quickjs/libregexp.c
  libs/quickjs/libunicode.c
  libs/quickjs/cutils.c
  libs/quickjs/dtoa.c
  ${QUICKJS_LIBC}
  ${WASM3_SRC}
)

if(NOT MSVC)
  target_link_libraries(libpapagaio PRIVATE m)
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

