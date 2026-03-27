CC = gcc
CFLAGS = -O2 -Wall -fPIC -I./quickjs -D_GNU_SOURCE -DCONFIG_VERSION=\"$(shell cat quickjs/VERSION)\" -fvisibility=hidden
LDFLAGS = -shared -lm -ldl -lpthread

QJS_DIR = ./quickjs
QJS_OBJS = $(QJS_DIR)/quickjs.o $(QJS_DIR)/dtoa.o $(QJS_DIR)/libregexp.o $(QJS_DIR)/libunicode.o $(QJS_DIR)/cutils.o

TARGET = qjs.so
SRC = qjs_plugin.c

all: $(TARGET)

$(QJS_DIR)/%.o: $(QJS_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(SRC) $(QJS_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(QJS_OBJS) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(QJS_OBJS)
