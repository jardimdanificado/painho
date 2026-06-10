CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra -fPIC -I../../src
LDFLAGS ?= -shared -ltcc

TARGET = papagaio_tcc.so

all: $(TARGET)

$(TARGET): papagaio_tcc.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
