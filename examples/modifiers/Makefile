CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra -fPIC -I../../src
LDFLAGS ?= -shared

SRCS = $(wildcard *.c)
TARGETS = $(SRCS:.c=.so)

all: $(TARGETS)

%.so: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean
