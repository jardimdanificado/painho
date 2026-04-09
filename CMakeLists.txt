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
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    
    # Configure paths for resources
    # We copy them to the binary dir to simplify RC inclusion
    configure_file(lib/wasm-libc/include/papagaio_wasm.h ${CMAKE_CURRENT_BINARY_DIR}/papagaio_wasm.h COPYONLY)
    configure_file(lib/wasm-libc/preprocessor.pap ${CMAKE_CURRENT_BINARY_DIR}/preprocessor.pap COPYONLY)
    
    # Check if we should embed clang (provided via -DCLANG_PATH=...)
    if (CLANG_PATH)
        add_definitions(-DEMBED_CLANG)
        configure_file(${CLANG_PATH} ${CMAKE_CURRENT_BINARY_DIR}/clang.exe COPYONLY)
    endif()

    add_executable(papagaiocc src/papagaiocc_win.c src/papagaiocc.rc)
    add_dependencies(papagaiocc papagaio)
    
    # For RC compiler to find files in the binary directory
    set_target_properties(papagaiocc PROPERTIES COMPILE_FLAGS "/I${CMAKE_CURRENT_BINARY_DIR}")
    
    # Ensure papagaio.exe is built and accessible in a fixed location for RC
    add_custom_command(TARGET papagaio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:papagaio> ${CMAKE_CURRENT_BINARY_DIR}/papagaio.exe)

    # Fix for MSVC RC searching in binary dir
    if (MSVC)
        set_property(SOURCE src/papagaiocc.rc PROPERTY COMPILE_FLAGS "/I${CMAKE_CURRENT_BINARY_DIR}")
    endif()

    target_link_libraries(papagaiocc libpapagaio)
    install(TARGETS papagaiocc DESTINATION bin)
endif()
