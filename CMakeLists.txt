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

# Plugins (Optional)
option(BUILD_PLUGINS "Build plugins" ON)
if (BUILD_PLUGINS)
  # Lua Plugin
  if (EXISTS ${CMAKE_SOURCE_DIR}/plugins/lua/lua_plugin.c)
    add_library(lua MODULE plugins/lua/lua_plugin.c)
    target_link_libraries(lua libpapagaio)
    set_target_properties(lua PROPERTIES PREFIX "")
  endif()

  # Micro QuickJS Plugin
  if (EXISTS ${CMAKE_SOURCE_DIR}/plugins/mquickjs/mjs_plugin.c)
    # This is a bit complex due to the generator, but let's try a basic setup
    add_library(mqjs MODULE plugins/mquickjs/mjs_plugin.c 
                plugins/mquickjs/mquickjs/mquickjs.c
                plugins/mquickjs/mquickjs/dtoa.c
                plugins/mquickjs/mquickjs/libm.c
                plugins/mquickjs/mquickjs/cutils.c)
    target_include_directories(mqjs PRIVATE plugins/mquickjs)
    target_link_libraries(mqjs libpapagaio)
    set_target_properties(mqjs PROPERTIES PREFIX "")
    if (MSVC)
       target_compile_definitions(mqjs PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
  endif()

  # QuickJS Plugin
  if (EXISTS ${CMAKE_SOURCE_DIR}/plugins/quickjs/qjs_plugin.c)
    add_library(qjs MODULE plugins/quickjs/qjs_plugin.c
                plugins/quickjs/quickjs/quickjs.c
                plugins/quickjs/quickjs/dtoa.c
                plugins/quickjs/quickjs/libregexp.c
                plugins/quickjs/quickjs/libunicode.c
                plugins/quickjs/quickjs/cutils.c)
    target_include_directories(qjs PRIVATE plugins/quickjs/quickjs)
    target_link_libraries(qjs libpapagaio)
    set_target_properties(qjs PROPERTIES PREFIX "" COMPILE_DEFINITIONS "CONFIG_VERSION=\"2021-03-27\"")
    if (MSVC)
       target_compile_definitions(qjs PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
  endif()
endif()
