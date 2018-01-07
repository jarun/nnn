VERSION = 1.6

PREFIX ?= /usr/local
MANPREFIX ?= $(PREFIX)/share/man
STRIP ?= strip

CFLAGS ?= -O3
CFLAGS += -Wall -Wextra -Wno-unused-parameter
LDLIBS = -lreadline

ifeq ($(shell pkg-config ncursesw && echo 1),1)
	CFLAGS += $(shell pkg-config --cflags ncursesw)
	LDLIBS += $(shell pkg-config --libs   ncursesw)
else
	LDLIBS += -lncurses
endif

DISTFILES = nlay nnn.c nnn.h nnn.1 Makefile README.md LICENSE
SRC = nnn.c
BIN = nnn
PLAYER = nlay

all: $(BIN) $(PLAYER)

$(SRC): nnn.h

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

debug: $(SRC)
	$(CC) -DDEBUGMODE -g $(CFLAGS) $(LDFLAGS) -o $(BIN) $^ $(LDLIBS)

install: all
	install -m 0755 -d $(DESTDIR)$(PREFIX)/bin
	install -m 0755 -t $(DESTDIR)$(PREFIX)/bin $(BIN) $(PLAYER)
	install -m 0755 -d $(DESTDIR)$(MANPREFIX)/man1
	install -m 0644 -t $(DESTDIR)$(MANPREFIX)/man1 $(BIN).1

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(BIN)
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(PLAYER)
	$(RM) $(DESTDIR)$(MANPREFIX)/man1/$(BIN).1

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

.PHONY: all debug install uninstall strip dist clean
