BIN=bin

WARN=-Wall -Wextra
DEBUG=-Og -ggdb3 -fsanitize=address -fsanitize=pointer-compare \
-fsanitize=pointer-subtract -fsanitize=undefined \
-fsanitize-address-use-after-scope -fstack-check \
-fno-stack-clash-protection
CFLAGS=$(DEBUG) $(WARN) -march=native -std=gnu99
LIBS=-lxcb -lxcb-randr
RELEASE=-O2 -s -pipe -flto=4
CFLAGS=$(WARN) -march=native -std=gnu99 $(LIBS)

SOURCE=$(wildcard *.c)
$(VERBOSE).SILENT:

main: main.c config.c global.c window.c
	$(CC) $(DEBUG) $(CFLAGS) -o $(BIN)/$@ $^

clean:
	rm -f $(wildcard bin/*)

install:
	cp bin/main /home/paczur/main

uninstall:
	rm /home/paczur/main
	rm /home/paczur/out

test: uninstall install
