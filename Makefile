# papagaio — Lua pattern-matching / text-processing module
# ========================================================
#
# Targets:
#   make            build papagaio.so (default Lua 5.4)
#   make LUA=luajit build for LuaJIT
#   make test       build + run teste.lua
#   make install    copy .so to LUA_LIBDIR (user-local by default)
#   make clean      remove build artifacts
#
CC         ?= cc
AR         ?= ar
RANLIB     ?= ranlib

# ── Platform ────────────────────────────────────────────────────────────
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  SHARED_FLAGS = -shared -fPIC -undefined dynamic_lookup
  SO_EXT       = .so
else
  SHARED_FLAGS = -shared -fPIC
  SO_EXT       = .so
endif

CFLAGS  ?= -O2 -Wall -Wextra -std=c99
LDFLAGS ?=

TARGET_SO = papagaio$(SO_EXT)
TARGET_A  = libpapagaio.a
TARGET_BIN = papagaio
SRC       = papagaio.c
OBJ       = papagaio.o

LUA_BIN    ?= lua
PREFIX     ?= $(HOME)/.local
BINDIR     ?= $(PREFIX)/bin
LUA_LIBDIR ?= $(PREFIX)/lib/lua/5.4
LIBDIR     ?= $(PREFIX)/lib
INCDIR     ?= $(PREFIX)/include

# ── Targets ─────────────────────────────────────────────────────────────
.PHONY: all clean test install static

all: $(TARGET_SO) $(TARGET_A) $(TARGET_BIN)

$(OBJ): $(SRC) papagaio.h
	$(CC) -c $(CFLAGS) -fPIC -o $@ $<

$(TARGET_SO): $(OBJ)
	$(CC) $(SHARED_FLAGS) -o $@ $(OBJ) $(LDFLAGS) -lm

$(TARGET_A): $(OBJ)
	$(AR) rcs $@ $(OBJ)
	$(RANLIB) $@

$(TARGET_BIN): main.c $(TARGET_A)
	$(CC) $(CFLAGS) -o $@ main.c $(TARGET_A) $(LDFLAGS) -lm

static: $(TARGET_A)

test: $(TARGET_SO)
	@if [ -z "$(LUA_BIN)" ]; then echo "Lua not found in PATH"; exit 1; fi
	@echo "=== papagaio test ==="
	LUA_CPATH="./?.so;;" $(LUA_BIN) teste.lua

install: $(TARGET_SO) $(TARGET_A) $(TARGET_BIN)
	install -d $(BINDIR)
	install -m 755 $(TARGET_BIN) $(BINDIR)/$(TARGET_BIN)
	install -d $(LUA_LIBDIR)
	install -m 755 $(TARGET_SO) $(LUA_LIBDIR)/$(TARGET_SO)
	install -d $(LIBDIR)
	install -m 644 $(TARGET_A) $(LIBDIR)/$(TARGET_A)
	install -d $(INCDIR)
	install -m 644 papagaio.h $(INCDIR)/papagaio.h
	@echo "Installed papagaio to $(PREFIX)/"

clean:
	rm -f $(TARGET_SO) $(TARGET_A) $(TARGET_BIN) $(OBJ)
	rm -rf dist/
	rm -rf obsidian-plugin/dist obsidian-plugin/node_modules

wasm: papagaio.c papagaio.h
	mkdir -p dist/wasm
	emcc -O3 $(CFLAGS) papagaio.c -o dist/wasm/papagaio_wasm.js \
		-s WASM=1 \
		-s MODULARIZE=1 \
		-s EXPORT_ES6=1 \
		-s SINGLE_FILE=1 \
		-s "EXPORTED_FUNCTIONS=['_papagaio_open', '_papagaio_close', '_papagaio_process_text', '_papagaio_cleanup', '_malloc', '_free']" \
		-s "EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap', 'UTF8ToString', 'stringToUTF8', 'lengthBytesUTF8']" \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s NODEJS_CATCH_EXIT=0 \
		-s NODEJS_CATCH_REJECTION=0
	cp papagaio.js dist/wasm/papagaio.js

obsidian: wasm
	cd obsidian-plugin && npm install && npm run build
	mkdir -p dist/papagaio-md
	cp obsidian-plugin/dist/main.js dist/papagaio-md/main.js
	cp obsidian-plugin/manifest.json dist/papagaio-md/manifest.json
	cp obsidian-plugin/styles.css dist/papagaio-md/styles.css
