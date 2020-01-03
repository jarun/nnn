VERSION = $(shell grep -m1 VERSION $(SRC) | cut -f 2 -d'"')

PREFIX ?= /boot/system/non-packaged
MANPREFIX ?= $(PREFIX)/documentation/man
STRIP ?= strip
PKG_CONFIG ?= pkg-config
INSTALL ?= install
CP ?= cp

CFLAGS_OPTIMIZATION ?= -O3

O_DEBUG := 0
O_NORL := 0  # no readline support
O_NOLOC := 0  # no locale support

# convert targets to flags for backwards compatibility
ifneq ($(filter debug,$(MAKECMDGOALS)),)
	O_DEBUG := 1
endif
ifneq ($(filter norl,$(MAKECMDGOALS)),)
	O_NORL := 1
endif
ifneq ($(filter noloc,$(MAKECMDGOALS)),)
	O_NORL := 1
	O_NOLOC := 1
endif

ifeq ($(O_DEBUG),1)
	CPPFLAGS += -DDBGMODE
	CFLAGS += -g
	LDLIBS += -lrt
endif

ifeq ($(O_NORL),1)
	CPPFLAGS += -DNORL
else
	LDLIBS += -lreadline
endif

ifeq ($(O_NOLOC),1)
	CPPFLAGS += -DNOLOCALE
endif

ifeq ($(shell $(PKG_CONFIG) ncursesw && echo 1),1)
	CFLAGS_CURSES ?= $(shell $(PKG_CONFIG) --cflags ncursesw)
	LDLIBS_CURSES ?= $(shell $(PKG_CONFIG) --libs   ncursesw)
else ifeq ($(shell $(PKG_CONFIG) ncurses && echo 1),1)
	CFLAGS_CURSES ?= $(shell $(PKG_CONFIG) --cflags ncurses)
	LDLIBS_CURSES ?= $(shell $(PKG_CONFIG) --libs   ncurses)
else
	LDLIBS_CURSES ?= -lncurses
endif

ifeq ($(shell uname -s), Haiku)
	LDLIBS_HAIKU ?= -lstdc++ -lbe
	SRC_HAIKU ?= misc/haiku/nm.cpp
	OBJS_HAIKU ?= misc/haiku/nm.o
endif

CFLAGS += -Wall -Wextra
CFLAGS += $(CFLAGS_OPTIMIZATION)
CFLAGS += $(CFLAGS_CURSES)

LDLIBS += $(LDLIBS_CURSES) $(LDLIBS_HAIKU)

DISTFILES = src nnn.1 Makefile README.md LICENSE
SRC = src/nnn.c
HEADERS = src/nnn.h
BIN = nnn
OBJS := nnn.o $(OBJS_HAIKU)

all: $(BIN)

ifeq ($(shell uname -s), Haiku)
$(OBJS_HAIKU): $(SRC_HAIKU)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<
endif

nnn.o: $(SRC) $(HEADERS)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# targets for backwards compatibility
debug: $(BIN)
norl: $(BIN)
noloc: $(BIN)

install: all
	$(INSTALL) -m 0755 -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 $(BIN) $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) -m 0755 -d $(DESTDIR)$(MANPREFIX)/man1
	$(INSTALL) -m 0644 $(BIN).1 $(DESTDIR)$(MANPREFIX)/man1

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(BIN)
	$(RM) $(DESTDIR)$(MANPREFIX)/man1/$(BIN).1

strip: $(BIN)
	$(STRIP) $^

dist:
	mkdir -p nnn-$(VERSION)
	$(CP) -r $(DISTFILES) nnn-$(VERSION)
	mkdir -p nnn-$(VERSION)/misc
	$(CP) -r misc/haiku nnn-$(VERSION)/misc
	tar -cf - nnn-$(VERSION) | gzip > nnn-$(VERSION).tar.gz
	$(RM) -r nnn-$(VERSION)

clean:
	$(RM) -f $(BIN) $(OBJS) nnn-$(VERSION).tar.gz

skip: ;

.PHONY: all debug install uninstall strip dist clean
