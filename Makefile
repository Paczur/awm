BIN=bin
BUILD=build
DIRS=$(BIN) $(BUILD)
SRC=src

WARN=-Wall -Wextra
VERBOSITY=-D DEBUG -D HINT_DEBUG -D LAYOUT_TRACE -D SYSTEM_DEBUG -D LAYOUT_GRID_TRACE
MEMORY_DEBUG=-fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract
DEBUG=$(VERBOSITY) -Og -ggdb3  -fsanitize=undefined \
			-fsanitize-address-use-after-scope -fstack-check -fno-stack-clash-protection
TEXT=$(shell pkg-config --cflags --libs pangocairo fontconfig)
X=$(shell pkg-config --cflags --libs xcb xcb-randr xcb-xkb xkbcommon-x11 xcb-icccm)
THREADS=-lpthread
LIBS= $(X) $(TEXT) $(THREADS)
RELEASE=-O2 -s -pipe -flto=4 -fwhole-program
CFLAGS=$(WARN) -march=native -std=gnu99 $(LIBS)
SOURCES=$(wildcard $(SRC)/*.c $(SRC)/**/*.c)
OBJECTS=$(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(SOURCES))
DEPENDS=$(patsubst $(SRC)/%.c,$(BUILD)/%.d,$(SOURCES))

export CCACHE_DIR := ccache
CC=ccache gcc

all: release

$(shell mkdir -p $(dir $(DEPENDS)))
-include $(DEPENDS)

.PHONY: all install uninstall release debug re re_clean clean
MAKEFLAGS := --jobs=$(shell nproc)
MAKEFLAGS += --output-sync=target
$(VERBOSE).SILENT:

install: $(BIN)/awm
	cp $(BIN)/awm /usr/bin
	mkdir /etc/awm
	cp -r scripts /etc/awm

uninstall:
	rm /usr/bin/awm
	rm -r /etc/awm

re: re_clean debug
re_clean:
	rm -rf $(BIN)/out

release: CFLAGS += $(RELEASE)
release: binaries

debug: CFLAGS += $(DEBUG)
debug: binaries

clean:
	rm -rf $(BIN) $(BUILD)

clean_all:
	rm -rf $(BIN) $(BUILD) $(CCACHE_DIR)

binaries: $(BIN)/awm

$(BIN):
	mkdir -p $(BIN)

$(BIN)/awm: $(OBJECTS) | $(BIN)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD):
	mkdir -p $(dir $(OBJECTS)) $(dir $(DEPENDS))

$(BUILD)/%.o: $(SRC)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<
