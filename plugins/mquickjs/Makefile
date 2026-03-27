# Makefile for Papagaio mquickjs Plugin
CC = gcc
HOST_CC = gcc
CFLAGS = -O2 -Wall -fPIC -I./mquickjs -D_GNU_SOURCE -fvisibility=hidden
LDFLAGS = -shared -lm

MJS_DIR = ./mquickjs
MQJS_OBJS = $(MJS_DIR)/mquickjs.o $(MJS_DIR)/dtoa.o $(MJS_DIR)/libm.o $(MJS_DIR)/cutils.o

TARGET = mjs.so
SRC = mjs_plugin.c

all: $(TARGET)

# Build mquickjs objects
$(MJS_DIR)/%.o: $(MJS_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Generator for atoms
mjs_stdlib_gen: mjs_stdlib.host.o $(MJS_DIR)/mquickjs_build.host.o
	$(HOST_CC) -o $@ $^

mjs_stdlib.host.o: mjs_stdlib.c
	$(HOST_CC) $(CFLAGS) -c -o $@ $<

$(MJS_DIR)/mquickjs_build.host.o: $(MJS_DIR)/mquickjs_build.c
	$(HOST_CC) $(CFLAGS) -c -o $@ $<

# Generate atoms and stdlib headers
$(MJS_DIR)/mquickjs_atom.h: mjs_stdlib_gen
	./mjs_stdlib_gen -a > $@

mjs_stdlib.h: mjs_stdlib_gen
	./mjs_stdlib_gen > $@

# Plugin build
$(MJS_DIR)/mquickjs.o: $(MJS_DIR)/mquickjs.c $(MJS_DIR)/mquickjs_atom.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(SRC) $(MQJS_OBJS) mjs_stdlib.h $(MJS_DIR)/mquickjs_atom.h
	$(CC) $(CFLAGS) -o $@ $(SRC) $(MQJS_OBJS) $(LDFLAGS)

clean:
	rm -f $(TARGET) mjs_stdlib.h mjs_stdlib_gen mjs_stdlib.host.o $(MJS_DIR)/*.o $(MJS_DIR)/*.host.o
