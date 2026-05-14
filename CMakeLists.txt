cmake_minimum_required(VERSION 3.10)
project(papagaio C)

set(CMAKE_C_STANDARD 99)

# Parse version from package.json
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/package.json" PACKAGE_JSON_CONTENT)
string(REGEX MATCH "\"version\": \"([0-9.]+)\"" _ "${PACKAGE_JSON_CONTENT}")
set(PAPAGAIO_VERSION ${CMAKE_MATCH_1})
add_definitions(-DCONFIG_VERSION="${PAPAGAIO_VERSION}")

include_directories(. src)

# Main library (static)
if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
endif()

add_library(libpapagaio STATIC
  src/papagaio.c
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

