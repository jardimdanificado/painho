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

# Standalone Compiler Driver (Windows only for now via CMake)
if (WIN32)
    set(PAP_VERSION "0.26.1")
    add_definitions(-DPAP_VERSION="${PAP_VERSION}")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    
    add_executable(papagaiocc src/papagaiocc_win.c src/papagaiocc.rc)
    add_dependencies(papagaiocc papagaio)
    
    # Pass paths to the Resource Compiler
    # We use unquoted definitions because papagaiocc.rc uses TOSTRING() logic
    set(SDK_H_PATH "${CMAKE_CURRENT_SOURCE_DIR}/examples/lib.c")
    set(PRE_PAP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/preprocessor.pap")
    
    set_property(SOURCE src/papagaiocc.rc PROPERTY COMPILE_DEFINITIONS
        "SDK_H_PATH=${SDK_H_PATH};PRE_PAP_PATH=${PRE_PAP_PATH};PAPAGAIO_EXE_PATH=$<TARGET_FILE:papagaio>")

    if (CLANG_PATH)
        add_definitions(-DEMBED_CLANG)
        set_property(SOURCE src/papagaiocc.rc APPEND PROPERTY COMPILE_DEFINITIONS "CLANG_EXE_PATH=${CLANG_PATH}")
    endif()

    target_link_libraries(papagaiocc libpapagaio)
    install(TARGETS papagaiocc DESTINATION bin)
endif()
