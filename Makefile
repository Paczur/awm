BIN=bin

WARN=-Wall -Wextra
DEBUG=-D DEBUG -Og -ggdb3 -fsanitize=address -fsanitize=pointer-compare \
-fsanitize=pointer-subtract -fsanitize=undefined \
-fsanitize-address-use-after-scope -fstack-check \
-fno-stack-clash-protection
TEXT=$(shell pkg-config --cflags --libs pangocairo fontconfig)
X=$(shell pkg-config --cflags --libs x11-xcb xcb-randr)
THREADS=-lpthread
LIBS= $(X) $(TEXT) $(THREADS)
RELEASE=-O2 -s -pipe -flto=4
CFLAGS=$(WARN) -march=native -std=gnu99 $(LIBS)

SOURCE=$(wildcard *.c)
$(VERBOSE).SILENT:

release: CFLAGS += $(RELEASE)
release: main

debug: CFLAGS += $(DEBUG)
debug: main

main: $(SOURCE)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

clean:
	rm -f $(wildcard bin/*)

test: clean debug
