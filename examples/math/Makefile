CC = gcc
CFLAGS = -Wall -Wextra -fPIC -O2 -I../../src -I../../src/louro -I../../src/louro/libs
LDFLAGS = -shared

all: papagaio_math.so

papagaio_math.so: papagaio_math.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f papagaio_math.so
