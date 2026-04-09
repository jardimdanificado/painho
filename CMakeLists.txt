cmake_minimum_required(VERSION 3.10)
project(papagaio C)

set(CMAKE_C_STANDARD 99)

include_directories(.)

file(GLOB WASM3_SRC "lib/wasm3/*.c")

# Main library (static)
add_library(libpapagaio STATIC
  src/papagaio.c
  lib/libregexp/libregexp.c
  lib/libregexp/cutils.c
  lib/libregexp/libunicode.c
  ${WASM3_SRC}
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

# Standalone Compiler Driver (Windows only for now via CMake)
if (WIN32)
    set(PAP_VERSION "0.24.1")
    add_definitions(-DPAP_VERSION="${PAP_VERSION}")
    
    # Check if we should embed clang (provided via -DCLANG_PATH=...)
    if (CLANG_PATH)
        add_definitions(-DEMBED_CLANG)
        configure_file(${CLANG_PATH} ${CMAKE_BINARY_DIR}/clang.exe COPYONLY)
    endif()

    add_executable(papagaiocc src/papagaiocc_win.c src/papagaiocc.rc)
    target_link_libraries(papagaiocc libpapagaio)
    install(TARGETS papagaiocc DESTINATION bin)
endif()
