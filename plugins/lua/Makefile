# Makefile for Papagaio Lua Plugin
# Compiles lua.so with minilua.h statically linked by default
CC = cc
CFLAGS = -O2 -Wall -Wextra -fPIC -std=c99 -DLUA_IMPL
LDFLAGS = -shared -lm

TARGET = lua.so
SRC = lua_plugin.c

all: $(TARGET)

$(TARGET): $(SRC) minilua.h
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
