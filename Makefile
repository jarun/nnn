VERSION = 2.0

PREFIX ?= /usr/local
MANPREFIX ?= $(PREFIX)/share/man
STRIP ?= strip
PKG_CONFIG ?= pkg-config
INSTALL ?= install

CFLAGS ?= -O3
CFLAGS += -Wall -Wextra -Wno-unused-parameter

ifeq ($(shell $(PKG_CONFIG) ncursesw && echo 1),1)
	CFLAGS += $(shell $(PKG_CONFIG) --cflags ncursesw)
	LDLIBS += $(shell $(PKG_CONFIG) --libs   ncursesw)
else
	LDLIBS += -lncurses
endif

DISTFILES = nlay nlay.1 nnn.c nnn.h nnn.1 Makefile README.md LICENSE
SRC = nnn.c
BIN = nnn
PLAYER = nlay

all: $(BIN) $(PLAYER)

$(SRC): nnn.h

$(BIN): $(SRC)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

debug: $(SRC)
	$(CC) -DDEBUGMODE -g $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $(BIN) $^ $(LDLIBS)

install: all
	$(INSTALL) -m 0755 -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 $(BIN) $(PLAYER) $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 -d $(DESTDIR)$(MANPREFIX)/man1
	$(INSTALL) -m 0644 $(BIN).1 $(DESTDIR)$(MANPREFIX)/man1
	$(INSTALL) -m 0644 $(PLAYER).1 $(DESTDIR)$(MANPREFIX)/man1

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(BIN)
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(PLAYER)
	$(RM) $(DESTDIR)$(MANPREFIX)/man1/$(BIN).1
	$(RM) $(DESTDIR)$(MANPREFIX)/man1/$(PLAYER).1

strip: $(BIN)
	$(STRIP) $^

dist:
	mkdir -p nnn-$(VERSION)
	$(CP) $(DISTFILES) nnn-$(VERSION)
	tar -cf nnn-$(VERSION).tar nnn-$(VERSION)
	gzip nnn-$(VERSION).tar
	$(RM) -r nnn-$(VERSION)

clean:
	$(RM) -f $(BIN) nnn-$(VERSION).tar.gz

.PHONY: $(BIN) $(SRC) all debug install uninstall strip dist clean
