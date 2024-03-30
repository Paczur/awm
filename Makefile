BIN=bin
BUILD=build
DIRS=$(BIN) $(BUILD)
SRC=src
TOOLS=tools

WARN=-Wall -Wextra
VERBOSITY=-D DEBUG -D HINT_DEBUG -D LAYOUT_DEBUG -D SYSTEM_DEBUG
DEBUG=$(VERBOSITY) -Og -ggdb3 -fsanitize=address -fsanitize=pointer-compare \
-fsanitize=pointer-subtract -fsanitize=undefined \
-fsanitize-address-use-after-scope -fstack-check \
-fno-stack-clash-protection
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

all: test

$(shell mkdir -p $(dir $(DEPENDS)))
-include $(DEPENDS)

.PHONY: all release debug test test_clean clean
MAKEFLAGS := --jobs=$(shell nproc)
MAKEFLAGS += --output-sync=target
$(VERBOSE).SILENT:


test_clean:
	rm -rf $(BIN)/out
test: test_clean debug

release: CFLAGS += $(RELEASE)
release: binaries

debug: CFLAGS += $(DEBUG)
debug: binaries

clean:
	rm -rf $(BIN) $(BUILD)

clean_all:
	rm -rf $(BIN) $(BUILD) $(CCACHE_DIR)

binaries: $(BIN)/idkwm tools

$(BIN):
	mkdir -p $(BIN)

tools: $(BIN)/idkmsg

$(BIN)/idkmsg: $(TOOLS)/idkmsg.c | $(BIN)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/idkwm: $(OBJECTS) | $(BIN)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD):
	mkdir -p $(dir $(OBJECTS)) $(dir $(DEPENDS))

$(BUILD)/%.o: $(SRC)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<
