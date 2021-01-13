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
O_ICONS := 0  # support icons-in-terminal
O_NERD := 0  # support icons-nerdfont
O_QSORT := 0  # use Alexey Tourbin's QSORT implementation
O_BENCH := 0  # benchmark mode (stops at first user input)
O_NOSSN := 0  # enable session support
O_NOUG := 0  # disable user, group name in status bar

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

ifeq ($(strip $(O_DEBUG)),1)
	CPPFLAGS += -DDBGMODE
	CFLAGS += -g
endif

ifeq ($(strip $(O_NORL)),1)
	CPPFLAGS += -DNORL
else ifeq ($(strip $(O_STATIC)),1)
	CPPFLAGS += -DNORL
else
	LDLIBS += -lreadline
endif

ifeq ($(strip $(O_PCRE)),1)
	CPPFLAGS += -DPCRE
	LDLIBS += -lpcre
endif

ifeq ($(strip $(O_NOLOC)),1)
	CPPFLAGS += -DNOLOCALE
endif

ifeq ($(strip $(O_NOMOUSE)),1)
	CPPFLAGS += -DNOMOUSE
endif

ifeq ($(strip $(O_NOBATCH)),1)
	CPPFLAGS += -DNOBATCH
endif

ifeq ($(strip $(O_NOFIFO)),1)
	CPPFLAGS += -DNOFIFO
endif

ifeq ($(strip $(O_CTX8)),1)
	CPPFLAGS += -DCTX8
endif

ifeq ($(strip $(O_ICONS)),1)
	CPPFLAGS += -DICONS
endif

ifeq ($(strip $(O_NERD)),1)
	CPPFLAGS += -DNERD
endif

ifeq ($(strip $(O_QSORT)),1)
	CPPFLAGS += -DTOURBIN_QSORT
endif

ifeq ($(strip $(O_BENCH)),1)
	CPPFLAGS += -DBENCH
endif

ifeq ($(strip $(O_NOSSN)),1)
	CPPFLAGS += -DNOSSN
endif

ifeq ($(strip $(O_NOUG)),1)
	CPPFLAGS += -DNOUG
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
ifeq ($(strip $(O_STATIC)),1)
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

upx: $(BIN)
	$(STRIP) $^
	upx -qqq $^

static:
	# regular static binary
	make O_STATIC=1 strip
	mv $(BIN) $(BIN)-static
	# static binary with icons-in-terminal support
	make O_STATIC=1 O_ICONS=1 strip
	mv $(BIN) $(BIN)-icons-static
	# static binary with patched nerd font support
	make O_STATIC=1 O_NERD=1 strip
	mv $(BIN) $(BIN)-nerd-static

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
	# upload sign file
	curl -XPOST 'https://uploads.github.com/repos/jarun/nnn/releases/$(ID)/assets?name=nnn-$(VERSION).tar.gz.sig' \
	    -H 'Authorization: token $(NNN_SIG_UPLOAD_TOKEN)' -H 'Content-Type: application/pgp-signature' \
	    --upload-file nnn-$(VERSION).tar.gz.sig
	tar -zcf $(BIN)-static-$(VERSION).x86_64.tar.gz $(BIN)-static
	# upx compress all static binaries
	upx -qqq $(BIN)-static
	upx -qqq $(BIN)-icons-static
	upx -qqq $(BIN)-nerd-static
	# upload static binary
	curl -XPOST 'https://uploads.github.com/repos/jarun/nnn/releases/$(ID)/assets?name=$(BIN)-static-$(VERSION).x86_64.tar.gz' \
	    -H 'Authorization: token $(NNN_SIG_UPLOAD_TOKEN)' -H 'Content-Type: application/x-sharedlib' \
	    --upload-file $(BIN)-static-$(VERSION).x86_64.tar.gz
	tar -zcf $(BIN)-icons-static-$(VERSION).x86_64.tar.gz $(BIN)-icons-static
	# upload icons-in-terminal compiled static binary
	curl -XPOST 'https://uploads.github.com/repos/jarun/nnn/releases/$(ID)/assets?name=$(BIN)-icons-static-$(VERSION).x86_64.tar.gz' \
	    -H 'Authorization: token $(NNN_SIG_UPLOAD_TOKEN)' -H 'Content-Type: application/x-sharedlib' \
	    --upload-file $(BIN)-icons-static-$(VERSION).x86_64.tar.gz
	# upload patched nerd font compiled static binary
	tar -zcf $(BIN)-nerd-static-$(VERSION).x86_64.tar.gz $(BIN)-nerd-static
	curl -XPOST 'https://uploads.github.com/repos/jarun/nnn/releases/$(ID)/assets?name=$(BIN)-nerd-static-$(VERSION).x86_64.tar.gz' \
	    -H 'Authorization: token $(NNN_SIG_UPLOAD_TOKEN)' -H 'Content-Type: application/x-sharedlib' \
	    --upload-file $(BIN)-nerd-static-$(VERSION).x86_64.tar.gz

clean:
	$(RM) -f $(BIN) nnn-$(VERSION).tar.gz *.sig $(BIN)-static $(BIN)-static-$(VERSION).x86_64.tar.gz $(BIN)-icons-static $(BIN)-icons-static-$(VERSION).x86_64.tar.gz $(BIN)-nerd-static $(BIN)-nerd-static-$(VERSION).x86_64.tar.gz

skip: ;

.PHONY: all install uninstall strip static dist sign upload-local clean install-desktop uninstall-desktop
