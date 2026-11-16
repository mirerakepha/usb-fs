CC ?= gcc
CFLAGS ?= -Os -Wall -Wextra -ffunction-sections -fdata-sections
CFLAGS += $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS += -Wl,--gc-sections -Wl,--as-needed -s
LDLIBS := $(shell pkg-config --libs gtk+-3.0)

usb_fs: main.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f usb_fs

.PHONY: clean
