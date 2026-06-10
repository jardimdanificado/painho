CC ?= gcc
CFLAGS = -shared -fPIC -I../../src -DENV_HEADER='"my_env.h"'
TARGET = aot_plugin.so
SRC = aot_plugin.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
