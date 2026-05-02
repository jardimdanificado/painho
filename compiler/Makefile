CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wno-missing-field-initializers -Wno-empty-body -O2 -g3 \
	-Isrc/util -Isrc/cpp -Isrc/cc/frontend -D_DEFAULT_SOURCE -DXCC_TARGET_ARCH=XCC_ARCH_WASM

SRCS = \
	$(wildcard src/wcc/*.c) \
	$(wildcard src/cc/frontend/*.c) \
	src/util/archive.c \
	src/util/util.c src/util/table.c

OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: wcc

wcc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) wcc
