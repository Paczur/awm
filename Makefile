rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SOURCES=$(call rwildcard,src,*.c)
TESTS=$(call rwildcard,test,*.c)
LINKER_FLAGS=$(patsubst %.c, build/%.lf, $(TESTS))
SRC_OBJECTS=$(patsubst %.c, build/%.o, $(SOURCES))
TEST_OBJECTS=$(patsubst %.c, build/%.o, $(TESTS))
DEPENDENCIES=$(patsubst %.c, build/%.d, $(SOURCES)$(TESTS))
TEST_BIN=tests
TEST_RUN=build/$(TEST_BIN).run

MEMORY_DEBUG_FLAGS=-fsanitize=address -fsanitize=pointer-compare -fsanitize=pointer-subtract
WARN_FLAGS=-Wall -Wextra -Werror -Wno-error=cpp -Wno-unused-function -Wunused-result -Wvla -Wshadow -Wstrict-prototypes -Wno-maybe-uninitialized -Wno-logical-not-parentheses
SANTIIZER_FLAGS=-fsanitize=undefined -fsanitize-address-use-after-scope -fstack-check -fno-stack-clash-protection
DEBUG_FLAGS=$(WARN_FLAGS) $(MEMORY_DEBUG_FLAGS) $(SANITIZER_FLAGS) -Og -ggdb3
OPTIMIZE_FLAGS=-march=native -O2 -pipe -D NDEBUG
LINK_FLAGS=$(BASE_CFLAGS) -flto=4 -fwhole-program $(shell pkg-config --cflags --libs xcb xcb-randr xcb-xkb)
TEST_FLAGS=$(CFLAGS) -Isrc -lctf -Wno-unused-parameter
BASE_CFLAGS=-std=gnu11 -MMD -MP
CFLAGS=$(BASE_CFLAGS)

all: release

release: CFLAGS += $(OPTIMIZE_FLAGS)
release: bins

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TEST_RUN) bins

check: $(TEST_RUN)

clean:
	rm -rf build bin

bins: bin/awm

install: bins
	install -D bin/awm /usr/bin/awm-test
	mkdir -p /etc/awm
	cp -r scripts /etc/awm

uninstall:
	rm /usr/bin/awm

MAKEFLAGS += --no-builtin-rules
.SUFFIXES:
.DELETE_ON_ERROR:
.PHONY: all release debug check clean install uninstall
$(VERBOSE).SILENT:
$(shell mkdir -p $(dir $(DEPENDENCIES)))
-include $(DEPENDENCIES)

bin/awm: $(SRC_OBJECTS)
	mkdir -p $(@D)
	$(info LN  $@)
	$(CC) $(LINK_FLAGS) $(CFLAGS) -o $@ $^

$(TEST_RUN): bin/$(TEST_BIN)
	mkdir -p $(@D)
	$(info RUN $<)
	./$< --cleanup --sigsegv
	touch $@

bin/$(TEST_BIN): $(TEST_OBJECTS) $(filter-out build/src/awm.o, $(SRC_OBJECTS)) | build/test/$(TEST_BIN).lf
	mkdir -p $(@D)
	$(info LN  $@)
	$(CC) $(LINK_FLAGS) $(TEST_FLAGS) `cat $|` -o $@ $^

build/test/$(TEST_BIN).lf: $(TESTS)
	mkdir -p $(@D)
	$(info FLG $@ )
	$(SHELL) ./build_scripts/wraps $^ > $@

build/test/%.c: test/%.c
	mkdir -p $(@D)
	$(info E   $@)
	$(CC) $(TEST_FLAGS) -E -o $@ $<

build/test/%.o: test/%.c
	mkdir -p $(@D)
	$(info CC  $@)
	$(CC) $(TEST_FLAGS) -c -o $@ $<

build/test/main.o: test/main.c
	mkdir -p $(@D)
	$(info CC  $@)
	$(CC) $(TEST_FLAGS) -c -o $@ $<

build/src/%.o: src/%.c
	mkdir -p $(@D)
	$(info CC  $@)
	$(CC) $(CFLAGS) -c -o $@ $<
