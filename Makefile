BIN=bin
BUILD=build
DIRS=$(BIN) $(BUILD)
SRC=src

WARN=-Wall -Wextra
VALGRIND=-D DEBUG -Og -ggdb3
DEBUG=-D DEBUG -Og -ggdb3 -fsanitize=address -fsanitize=pointer-compare \
-fsanitize=pointer-subtract -fsanitize=undefined \
-fsanitize-address-use-after-scope -fstack-check \
-fno-stack-clash-protection
TEXT=$(shell pkg-config --cflags --libs pangocairo fontconfig)
X=$(shell pkg-config --cflags --libs x11-xcb xcb-randr)
THREADS=-lpthread
LIBS= $(X) $(TEXT) $(THREADS)
RELEASE=-O2 -s -pipe -flto=4 -fwhole-program
CFLAGS=$(WARN) -march=native -std=gnu99 $(LIBS)
SOURCES=$(wildcard $(SRC)/*.c $(SRC)/**/*.c)
OBJECTS=$(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(SOURCES))
DEPENDS=$(patsubst $(SRC)/%.c,$(BUILD)/%.d,$(SOURCES))

all: test

$(shell mkdir -p $(dir $(DEPENDS)))
-include $(DEPENDS)

.PHONY: all release debug test test_clean clean
$(VERBOSE).SILENT:

test: test_clean debug

release: CFLAGS += $(RELEASE)
release: $(BIN)/idkwm rename

debug: CFLAGS += $(DEBUG)
debug: $(BIN)/idkwm-debug

rename:
	mv $(BIN)/idkwm $(BIN)/idkwm-debug

valgrind: CFGLAS += $(VALGRIND)
valgrind: clean $(BIN)/idkwm-debug

clean:
	rm -rf $(BIN) $(BUILD)

test_clean:
	rm -rf $(BIN)/out

$(BIN):
	mkdir -p $(BIN)

$(BIN)/idkwm: $(SOURCES) | $(BIN)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/idkwm-debug: $(OBJECTS) | $(BIN)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD):
	mkdir -p $(dir $(OBJECTS)) $(dir $(DEPENDS))

$(BUILD)/%.o: $(SRC)/%.c | $(BUILD)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<
