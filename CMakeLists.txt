cmake_minimum_required(VERSION 3.10)
project(papagaio C)

set(CMAKE_C_STANDARD 99)

include_directories(.)

# Main library (static)
add_library(libpapagaio STATIC papagaio.c)
target_link_libraries(libpapagaio m)

# Main executable
add_executable(papagaio main.c)
target_link_libraries(papagaio libpapagaio m)

# Installation
install(TARGETS papagaio DESTINATION bin)
install(FILES papagaio.h DESTINATION include)
install(TARGETS libpapagaio DESTINATION lib)
