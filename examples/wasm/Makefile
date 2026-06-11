CC ?= gcc
CFLAGS = -O2 -Wall -Wextra -shared -fPIC -I../../src -Iwasm3/source -D_GNU_SOURCE
TARGET = papagaio_wasm.so
WASM3_SRC = $(wildcard wasm3/source/*.c)
SRC = wasm_plugin.c $(WASM3_SRC)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) -lm

clean:
	rm -f $(TARGET)
