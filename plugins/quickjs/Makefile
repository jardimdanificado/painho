# Makefile for Papagaio QuickJS Plugin
CC = cc
CFLAGS = -O2 -Wall -Wextra -fPIC -std=c99
LDFLAGS = -shared -lquickjs

TARGET = js.so
SRC = js_plugin.c

# Check for QuickJS headers
HAS_QUICKJS := $(shell cc -E -x c -o /dev/null - < /dev/null -include quickjs/quickjs.h >/dev/null 2>&1 && echo yes || echo no)

all:
ifeq ($(HAS_QUICKJS),yes)
	@$(MAKE) build
else
	@echo "QuickJS not found, skipping js.so build."
endif

build: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
