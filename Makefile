VERSION = 1.0

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CFLAGS += -O3 -march=native -Wall -Wextra -Wno-unused-parameter
LDLIBS = -lncursesw

DISTFILES = nnn.c config.def.h nnn.1 Makefile README.md LICENSE
LOCALCONFIG = config.h
SRC = nnn.c
BIN = nnn

all: $(BIN)

$(LOCALCONFIG): config.def.h
	cp config.def.h $@

$(SRC): $(LOCALCONFIG)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)
	strip $@

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f $(BIN).1 $(DESTDIR)$(MANPREFIX)/man1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)
	rm -f $(DESTDIR)$(MANPREFIX)/man1/$(BIN).1

dist:
	mkdir -p nnn-$(VERSION)
	cp $(DISTFILES) nnn-$(VERSION)
	tar -cf nnn-$(VERSION).tar nnn-$(VERSION)
	gzip nnn-$(VERSION).tar
	rm -rf nnn-$(VERSION)

clean:
	rm -f $(BIN) nnn-$(VERSION).tar.gz
