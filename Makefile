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
  EXE_EXT       =
else ifneq (,$(findstring MINGW,$(UNAME_S)))
  SHARED_FLAGS = -shared -fPIC
  SO_EXT       = .dll
  EXE_EXT       = .exe
else ifneq (,$(findstring MSYS,$(UNAME_S)))
  SHARED_FLAGS = -shared -fPIC
  SO_EXT       = .dll
  EXE_EXT       = .exe
else
  SHARED_FLAGS = -shared -fPIC
  SO_EXT       = .so
  EXE_EXT       =
endif

CFLAGS  ?= -O2 -Wall -Wextra -std=c99
ifneq (,$(findstring MINGW,$(UNAME_S))$(findstring MSYS,$(UNAME_S)))
  LDFLAGS = 
else
  LDFLAGS = -ldl
endif
LDFLAGS += -lm

TARGET_SO = papagaio$(SO_EXT)
TARGET_A  = libpapagaio.a
TARGET_BIN = papagaio$(EXE_EXT)
SRC       = src/papagaio.c lib/libregexp/libregexp.c lib/libregexp/cutils.c lib/libregexp/libunicode.c
OBJ       = $(SRC:.c=.o)

LUA_BIN    ?= lua
PREFIX     ?= $(HOME)/.local
BINDIR     ?= $(PREFIX)/bin
LUA_LIBDIR ?= $(PREFIX)/lib/lua/5.4
LIBDIR     ?= $(PREFIX)/lib
INCDIR     ?= $(PREFIX)/include

# ── Targets ─────────────────────────────────────────────────────────────
.PHONY: all clean test install static plugins

all: $(TARGET_SO) $(TARGET_A) $(TARGET_BIN) plugins

%.o: %.c
	$(CC) -c $(CFLAGS) -fPIC -o $@ $<

$(TARGET_SO): $(OBJ)
	$(CC) $(SHARED_FLAGS) -o $@ $(OBJ) $(LDFLAGS) -lm

$(TARGET_A): $(OBJ)
	$(AR) rcs $@ $(OBJ)
	$(RANLIB) $@

$(TARGET_BIN): src/main.c $(TARGET_A)
	$(CC) $(CFLAGS) -o $@ src/main.c $(TARGET_A) $(LDFLAGS) -lm

plugins:
	$(MAKE) -C plugins/lua
	$(MAKE) -C plugins/quickjs

static: $(TARGET_A)

test: test_c test_node

test_c: $(TARGET_A) plugins
	$(CC) $(CFLAGS) -o tests/test_bin tests/test.c $(TARGET_A) $(LDFLAGS) -lm
	@echo "=== Starting Papagaio C Tests ==="
	./tests/test_bin

test_node: wasm
	@if command -v node > /dev/null 2>&1; then \
		echo "=== Starting Papagaio Node.js Tests ==="; \
		node tests/test.js; \
	else \
		echo "Node.js not found, skipping Node tests."; \
	fi

install: $(TARGET_SO) $(TARGET_A) $(TARGET_BIN)
	install -d $(BINDIR)
	install -m 755 $(TARGET_BIN) $(BINDIR)/$(TARGET_BIN)
	install -d $(LUA_LIBDIR)
	install -m 755 $(TARGET_SO) $(LUA_LIBDIR)/$(TARGET_SO)
	install -d $(LIBDIR)
	install -m 644 $(TARGET_A) $(LIBDIR)/$(TARGET_A)
	install -d $(INCDIR)
	install -m 644 src/papagaio.h $(INCDIR)/src/papagaio.h
	@echo "Installed papagaio to $(PREFIX)/"

clean:
	rm -f $(TARGET_SO) $(TARGET_A) $(TARGET_BIN) $(OBJ)
	$(MAKE) -C plugins/lua clean
	$(MAKE) -C plugins/quickjs clean
	rm -rf dist/

wasm: src/papagaio.c src/papagaio.h
	mkdir -p dist/wasm
	emcc -O3 $(CFLAGS) src/papagaio.c lib/libregexp/libregexp.c lib/libregexp/cutils.c lib/libregexp/libunicode.c -o dist/wasm/papagaio_wasm.js \
		-s WASM=1 \
		-s MODULARIZE=1 \
		-s EXPORT_ES6=1 \
		-s SINGLE_FILE=1 \
		-s "EXPORTED_FUNCTIONS=['_papagaio_open', '_papagaio_close', '_papagaio_process_text', '_papagaio_register_command', '_papagaio_register_modifier', '_malloc', '_free']" \
		-s "EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap', 'getValue', 'UTF8ToString', 'stringToUTF8', 'lengthBytesUTF8', 'addFunction', 'removeFunction']" \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s ALLOW_TABLE_GROWTH=1 \
		-s NODEJS_CATCH_EXIT=0 \
		-s NODEJS_CATCH_REJECTION=0
	cp src/papagaio.js dist/wasm/papagaio.js
