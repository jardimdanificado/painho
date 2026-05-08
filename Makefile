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
WINDRES    ?= windres

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

PAP_VERSION = $(shell grep '"version":' package.json | cut -d '"' -f 4)
CFLAGS  ?= -O2 -Wall -Wextra -std=gnu99 -D_CRT_SECURE_NO_WARNINGS -Ilibs/quickjs -D_GNU_SOURCE -DCONFIG_VERSION=\"$(PAP_VERSION)\"
ifneq (,$(findstring MINGW,$(UNAME_S))$(findstring MSYS,$(UNAME_S)))
  LDFLAGS = 
else
  LDFLAGS = -ldl
endif
LDFLAGS += -lm

TARGET_SO = papagaio$(SO_EXT)
TARGET_A  = libpapagaio.a
TARGET_BIN = papagaio$(EXE_EXT)
WASM3_SRC = $(wildcard libs/wasm3/source/*.c)
QJS_SRC   = libs/quickjs/quickjs.c libs/quickjs/libregexp.c libs/quickjs/libunicode.c libs/quickjs/cutils.c libs/quickjs/dtoa.c libs/quickjs/quickjs-libc.c
SRC       = src/papagaio.c $(QJS_SRC) $(WASM3_SRC)
OBJ       = $(SRC:.c=.o)

LUA_BIN    ?= lua
PREFIX     ?= $(HOME)/.local
BINDIR     ?= $(PREFIX)/bin
LUA_LIBDIR ?= $(PREFIX)/lib/lua/5.4
LIBDIR     ?= $(PREFIX)/lib
INCDIR     ?= $(PREFIX)/include

# ── Targets ─────────────────────────────────────────────────────────────
.PHONY: all clean test install static

all: $(TARGET_SO) $(TARGET_A) $(TARGET_BIN)

%.o: %.c
	$(CC) -c $(CFLAGS) -fPIC -o $@ $<

$(TARGET_SO): $(OBJ)
	$(CC) $(SHARED_FLAGS) -o $@ $(OBJ) $(LDFLAGS)

$(TARGET_A): $(OBJ)
	$(AR) rcs $@ $(OBJ)
	$(RANLIB) $@

$(TARGET_BIN): src/main.c $(TARGET_A)
	$(CC) $(CFLAGS) -o $@ src/main.c $(TARGET_A) $(LDFLAGS)

static: $(TARGET_A)

test: test_c test_node

test_c: $(TARGET_A)
	$(CC) $(CFLAGS) -o tests/test_bin tests/test.c $(TARGET_A) $(LDFLAGS)
	@echo "=== Starting Papagaio C Tests ==="
	./tests/test_bin
	$(CC) $(CFLAGS) -o tests/test_priority_bin tests/test_priority.c $(TARGET_A) $(LDFLAGS)
	@echo "=== Starting Papagaio Priority Tests ==="
	./tests/test_priority_bin

test_node: wasm
	@if command -v node > /dev/null 2>&1; then \
		echo "=== Starting Papagaio Node.js Tests ==="; \
		node tests/test.js; \
	else \
		echo "Node.js not found, skipping Node tests."; \
	fi

test_valgrind: $(TARGET_A)
	$(CC) $(CFLAGS) -o tests/test_bin tests/test.c $(TARGET_A) $(LDFLAGS)
	@echo "=== Running Papagaio C Tests with Valgrind ==="
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./tests/test_bin
	$(CC) $(CFLAGS) -o tests/test_priority_bin tests/test_priority.c $(TARGET_A) $(LDFLAGS)
	@echo "=== Running Papagaio Priority Tests with Valgrind ==="
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./tests/test_priority_bin

bench: $(TARGET_BIN) wasm
	@echo "=== Running Benchmark (Native C) ==="
	@bash -c "time ./papagaio tests/benchmark.pap > /dev/null"
	@echo "=== Running Benchmark (Node.js WASM) ==="
	@bash -c "time ./bin/cli.js tests/benchmark.pap > /dev/null"

install: $(TARGET_SO) $(TARGET_A) $(TARGET_BIN)
	install -d $(BINDIR)
	install -m 755 $(TARGET_BIN) $(BINDIR)/$(TARGET_BIN)
	install -d $(LUA_LIBDIR)
	install -m 755 $(TARGET_SO) $(LUA_LIBDIR)/$(TARGET_SO)
	install -d $(LIBDIR)
	install -m 644 $(TARGET_A) $(LIBDIR)/$(TARGET_A)
	install -d $(INCDIR)
	install -m 644 src/papagaio.h $(INCDIR)/papagaio.h
	@echo "Installed papagaio to $(PREFIX)/"

papagaiocc: $(TARGET_BIN)
	@echo "▶ Generating papagaiocc (Standalone Compiler)..."
ifeq ($(EXE_EXT),.exe)
ifneq ($(CLANG_PATH),)
ifneq ($(CLANG_PATH),clang.exe)
	@cp "$(CLANG_PATH)" clang.exe
endif
endif
	$(WINDRES) $(if $(CLANG_PATH),-DEMBED_CLANG) src/papagaiocc.rc papagaiocc_res.o
	$(CC) $(CFLAGS) -DPAP_VERSION=\"0.26.1\" src/papagaiocc_win.c papagaiocc_res.o -o papagaiocc.exe
	@rm -f papagaiocc_res.o clang.exe
	@echo "✓ papagaiocc.exe successfully generated."
else
	@echo '#!/usr/bin/env bash' > papagaiocc
	@echo 'set -euo pipefail' >> papagaiocc
	@echo 'TMP_DIR=$$(mktemp -d /tmp/papagaiocc.XXXXXX)' >> papagaiocc
	@echo 'trap "rm -rf $$TMP_DIR" EXIT' >> papagaiocc
	@echo 'PAP_BIN="$$TMP_DIR/papagaio"' >> papagaiocc
ifneq ($(CLANG_PATH),)
	@echo 'CLANG_BIN="$$TMP_DIR/clang"' >> papagaiocc
else
	@echo 'CLANG_BIN="clang"' >> papagaiocc
endif
	@echo "base64 -d <<'EOF' > \"\$$PAP_BIN\"" >> papagaiocc
	@base64 < $(TARGET_BIN) >> papagaiocc
	@echo 'EOF' >> papagaiocc
	@echo 'chmod +x "$$PAP_BIN"' >> papagaiocc
ifneq ($(CLANG_PATH),)
	@echo "base64 -d <<'EOF' > \"\$$CLANG_BIN\"" >> papagaiocc
	@base64 < $(CLANG_PATH) >> papagaiocc
	@echo 'EOF' >> papagaiocc
	@echo 'chmod +x "$$CLANG_BIN"' >> papagaiocc
endif
	@echo 'ENTRY=""' >> papagaiocc
	@echo 'OUTPUT=""' >> papagaiocc
	@echo 'EXTRA_FLAGS=()' >> papagaiocc
	@echo 'while [[ $$# -gt 0 ]]; do' >> papagaiocc
	@echo '  case "$$1" in' >> papagaiocc
	@echo '    -o|--output) OUTPUT="$$2"; shift 2 ;;' >> papagaiocc
	@echo '    -*) EXTRA_FLAGS+=("$$1"); shift ;;' >> papagaiocc
	@echo '    *) if [[ -z "$$ENTRY" ]]; then ENTRY="$$1"; else EXTRA_FLAGS+=("$$1"); fi; shift ;;' >> papagaiocc
	@echo '  esac' >> papagaiocc
	@echo 'done' >> papagaiocc
	@echo 'if [[ -z "$$ENTRY" ]]; then echo "Usage: papagaiocc <input.c> [-o output.wasm] [clang flags...]"; exit 1; fi' >> papagaiocc
	@echo 'if [[ -z "$$OUTPUT" ]]; then BASENAME=$$(basename "$$ENTRY"); OUTPUT="$${BASENAME%.*}.wasm"; fi' >> papagaiocc
	@echo '"$$PAP_BIN" "$$ENTRY" "$${EXTRA_FLAGS[@]+$${EXTRA_FLAGS[@]}}" > "$$TMP_DIR/ready.c"' >> papagaiocc
	@echo '"$$CLANG_BIN" --target=wasm32-unknown-unknown -ffreestanding -nostdlib -fvisibility=hidden -Wl,--no-entry,--export-dynamic,--allow-undefined "$$TMP_DIR/ready.c" -o "$$OUTPUT" "$${EXTRA_FLAGS[@]+$${EXTRA_FLAGS[@]}}"' >> papagaiocc
	@chmod +x papagaiocc
	@echo "✓ papagaiocc successfully generated."
endif

clean:
	rm -f $(TARGET_SO) $(TARGET_A) $(TARGET_BIN) $(OBJ)
	rm -rf dist/

wasm: src/papagaio.c src/papagaio.h
	mkdir -p dist/wasm
	emcc -O3 $(CFLAGS) $(SRC) -o dist/wasm/papagaio_wasm.js \
		-s WASM=1 \
		-s MODULARIZE=1 \
		-s EXPORT_ES6=1 \
		-s SINGLE_FILE=1 \
		-s "EXPORTED_FUNCTIONS=['_papagaio_open', '_papagaio_close', '_papagaio_set_args', '_papagaio_process_text', '_papagaio_register_command', '_papagaio_register_modifier', '_malloc', '_free']" \
		-s "EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap', 'getValue', 'setValue', 'UTF8ToString', 'stringToUTF8', 'lengthBytesUTF8', 'addFunction', 'removeFunction']" \
		-s ALLOW_MEMORY_GROWTH=1 \
		-s ALLOW_TABLE_GROWTH=1 \
		-s NODEJS_CATCH_EXIT=0 \
		-s NODEJS_CATCH_REJECTION=0
	cp src/papagaio.js dist/wasm/papagaio.js
