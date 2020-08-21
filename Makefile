VERSION = $(shell grep -m1 VERSION $(SRC) | cut -f 2 -d'"')

PREFIX ?= /usr/local
MANPREFIX ?= $(PREFIX)/share/man
DESKTOPPREFIX ?= $(PREFIX)/share/applications
DESKTOPICONPREFIX ?= $(PREFIX)/share/icons/hicolor
STRIP ?= strip
PKG_CONFIG ?= pkg-config
INSTALL ?= install
CP ?= cp

CFLAGS_OPTIMIZATION ?= -O3

O_DEBUG := 0  # debug binary
O_NORL := 0  # no readline support
O_PCRE := 0  # link with PCRE library
O_NOLOC := 0  # no locale support
O_NOMOUSE := 0  # no mouse support
O_NOBATCH := 0  # no built-in batch renamer
O_NOFIFO := 0  # no FIFO previewer support
O_CTX8 := 0  # enable 8 contexts
O_ICONS := 0  # support icons
O_QSORT := 0  # use Alexey Tourbin's QSORT implementation
O_BENCH := 0  # benchmark mode (stops at first user input)

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
endif

ifeq ($(O_NORL),1)
	CPPFLAGS += -DNORL
else ifeq ($(O_STATIC),1)
	CPPFLAGS += -DNORL
else
	LDLIBS += -lreadline
endif

ifeq ($(O_PCRE),1)
	CPPFLAGS += -DPCRE
	LDLIBS += -lpcre
endif

ifeq ($(O_NOLOC),1)
	CPPFLAGS += -DNOLOCALE
endif

ifeq ($(O_NOMOUSE),1)
	CPPFLAGS += -DNOMOUSE
endif

ifeq ($(O_NOBATCH),1)
	CPPFLAGS += -DNOBATCH
endif

ifeq ($(O_NOFIFO),1)
	CPPFLAGS += -DNOFIFO
endif

ifeq ($(O_CTX8),1)
	CPPFLAGS += -DCTX8
endif

ifeq ($(O_ICONS),1)
	CPPFLAGS += -DICONS
endif

ifeq ($(O_QSORT),1)
	CPPFLAGS += -DTOURBIN_QSORT
endif

ifeq ($(O_BENCH),1)
	CPPFLAGS += -DBENCH
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

CFLAGS += -std=c11 -Wall -Wextra -Wshadow
CFLAGS += $(CFLAGS_OPTIMIZATION)
CFLAGS += $(CFLAGS_CURSES)

LDLIBS += $(LDLIBS_CURSES)

# static compilation needs libgpm development package
ifeq ($(O_STATIC),1)
	LDFLAGS += -static
	LDLIBS += -lgpm
endif

DISTFILES = src nnn.1 Makefile README.md LICENSE
SRC = src/nnn.c
HEADERS = src/nnn.h
BIN = nnn
DESKTOPFILE = misc/desktop/nnn.desktop
LOGOSVG = misc/logo/logo.svg
LOGO64X64 = misc/logo/logo-64x64.png

all: $(BIN)

$(BIN): $(SRC) $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

# targets for backwards compatibility
debug: $(BIN)
norl: $(BIN)
noloc: $(BIN)

install-desktop: $(DESKTOPFILE)
	$(INSTALL) -m 0755 -d $(DESTDIR)$(DESKTOPPREFIX)
	$(INSTALL) -m 0644 $(DESKTOPFILE) $(DESTDIR)$(DESKTOPPREFIX)
	$(INSTALL) -m 0755 -d $(DESTDIR)$(DESKTOPICONPREFIX)/scalable/apps
	$(INSTALL) -m 0644 $(LOGOSVG) $(DESTDIR)$(DESKTOPICONPREFIX)/scalable/apps/nnn.svg
	$(INSTALL) -m 0755 -d $(DESTDIR)$(DESKTOPICONPREFIX)/64x64/apps
	$(INSTALL) -m 0644 $(LOGO64X64) $(DESTDIR)$(DESKTOPICONPREFIX)/64x64/apps/nnn.png

uninstall-desktop:
	$(RM) $(DESTDIR)$(DESKTOPPREFIX)/$(DESKTOPFILE)
	$(RM) $(DESTDIR)$(DESKTOPICONPREFIX)/scalable/apps/nnn.svg
	$(RM) $(DESTDIR)$(DESKTOPICONPREFIX)/64x64/apps/nnn.png

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

static:
	make O_STATIC=1 strip
	mv $(BIN) $(BIN)-static

dist:
	mkdir -p nnn-$(VERSION)
	$(CP) -r $(DISTFILES) nnn-$(VERSION)
	tar -cf - nnn-$(VERSION) | gzip > nnn-$(VERSION).tar.gz
	$(RM) -r nnn-$(VERSION)

sign:
	git archive -o nnn-$(VERSION).tar.gz --format tar.gz --prefix=nnn-$(VERSION)/ v$(VERSION)
	gpg --detach-sign --yes nnn-$(VERSION).tar.gz
	rm -f nnn-$(VERSION).tar.gz

upload-local: sign static
	$(eval ID=$(shell curl -s 'https://api.github.com/repos/jarun/nnn/releases/tags/v$(VERSION)' | jq .id))
	curl -XPOST 'https://uploads.github.com/repos/jarun/nnn/releases/$(ID)/assets?name=nnn-$(VERSION).tar.gz.sig' \
	    -H 'Authorization: token $(NNN_SIG_UPLOAD_TOKEN)' -H 'Content-Type: application/pgp-signature' \
	    --upload-file nnn-$(VERSION).tar.gz.sig
	tar -zcf $(BIN)-static-$(VERSION).x86_64.tar.gz $(BIN)-static
	curl -XPOST 'https://uploads.github.com/repos/jarun/nnn/releases/$(ID)/assets?name=$(BIN)-static-$(VERSION).x86_64.tar.gz' \
	    -H 'Authorization: token $(NNN_SIG_UPLOAD_TOKEN)' -H 'Content-Type: application/x-sharedlib' \
	    --upload-file $(BIN)-static-$(VERSION).x86_64.tar.gz

clean:
	$(RM) -f $(BIN) nnn-$(VERSION).tar.gz *.sig $(BIN)-static $(BIN)-static-$(VERSION).x86_64.tar.gz

skip: ;

.PHONY: all install uninstall strip static dist sign upload-local clean install-desktop uninstall-desktop
