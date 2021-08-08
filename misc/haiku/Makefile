VERSION = $(shell grep -m1 VERSION $(SRC) | cut -f 2 -d'"')

PREFIX ?= /boot/system/non-packaged
MANPREFIX ?= $(PREFIX)/documentation/man
STRIP ?= strip
PKG_CONFIG ?= pkg-config
INSTALL ?= install
CP ?= cp

CFLAGS_OPTIMIZATION ?= -O3

O_DEBUG := 0  # debug binary
O_NORL := 0  # no readline support
O_PCRE := 0  # link with PCRE library
O_NOLC := 0  # no locale support
O_NOMOUSE := 0  # no mouse support
O_NOBATCH := 0  # no built-in batch renamer
O_NOFIFO := 0  # no FIFO previewer support
O_CTX8 := 0  # enable 8 contexts
O_ICONS := 0  # support icons-in-terminal
O_NERD := 0  # support icons-nerdfont
O_QSORT := 0  # use Alexey Tourbin's QSORT implementation
O_BENCH := 0  # benchmark mode (stops at first user input)
O_NOSSN := 0  # disable session support
O_NOUG := 0  # disable user, group name in status bar
O_NOX11 := 0  # disable X11 integration
O_MATCHFLTR := 0  # allow filters without matches

# User patches
O_GITSTATUS := 0 # add git status to detail view
O_NAMEFIRST := 0 # print file name first, add uid and guid to detail view

ifeq ($(strip $(O_GITSTATUS)),1)
	LDLIBS += -lgit2
endif

# convert targets to flags for backwards compatibility
ifneq ($(filter debug,$(MAKECMDGOALS)),)
	O_DEBUG := 1
endif
ifneq ($(filter norl,$(MAKECMDGOALS)),)
	O_NORL := 1
endif
ifneq ($(filter nolc,$(MAKECMDGOALS)),)
	O_NORL := 1
	O_NOLC := 1
endif

ifeq ($(strip $(O_DEBUG)),1)
	CPPFLAGS += -DDEBUG
	CFLAGS += -g
	LDLIBS += -lrt
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

ifeq ($(strip $(O_NOLC)),1)
	ifeq ($(strip $(O_ICONS)),1)
$(info *** Ignoring O_NOLC since O_ICONS is set ***)
	else ifeq ($(strip $(O_NERD)),1)
$(info *** Ignoring O_NOLC since O_NERD is set ***)
	else
		CPPFLAGS += -DNOLC
	endif
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

ifeq ($(strip $(O_NOX11)),1)
	CPPFLAGS += -DNOX11
endif

ifeq ($(strip $(O_MATCHFLTR)),1)
	CPPFLAGS += -DMATCHFLTR
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
	LDLIBS_HAIKU ?= -lstdc++ -lgnu -lbe
	SRC_HAIKU ?= misc/haiku/nm.cpp
	OBJS_HAIKU ?= misc/haiku/nm.o
endif

CFLAGS += -std=c11 -Wall -Wextra -Wshadow
CFLAGS += $(CFLAGS_OPTIMIZATION)
CFLAGS += $(CFLAGS_CURSES)

LDLIBS += $(LDLIBS_CURSES) -lpthread $(LDLIBS_HAIKU)
# static compilation needs libgpm development package
ifeq ($(strip $(O_STATIC)),1)
	LDFLAGS += -static
	LDLIBS += -lgpm
endif

DISTFILES = src nnn.1 Makefile README.md LICENSE
SRC = src/nnn.c
HEADERS = src/nnn.h
BIN = nnn
OBJS := nnn.o $(OBJS_HAIKU)

GITSTATUS = patches/gitstatus
NAMEFIRST = patches/namefirst

all: $(BIN)

ifeq ($(shell uname -s), Haiku)
$(OBJS_HAIKU): $(SRC_HAIKU)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<
endif

nnn.o: $(SRC) $(HEADERS)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

$(BIN): $(OBJS)
	@$(MAKE) --silent prepatch
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	@$(MAKE) --silent postpatch

# targets for backwards compatibility
debug: $(BIN)
norl: $(BIN)
nolc: $(BIN)

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
	# regular static binary
	make O_STATIC=1 strip
	mv $(BIN) $(BIN)-static

dist:
	mkdir -p nnn-$(VERSION)
	$(CP) -r $(DISTFILES) nnn-$(VERSION)
	mkdir -p nnn-$(VERSION)/misc
	$(CP) -r misc/haiku nnn-$(VERSION)/misc
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
	# upload static binary
	curl -XPOST 'https://uploads.github.com/repos/jarun/nnn/releases/$(ID)/assets?name=$(BIN)-static-$(VERSION).x86_64.tar.gz' \
	    -H 'Authorization: token $(NNN_SIG_UPLOAD_TOKEN)' -H 'Content-Type: application/x-sharedlib' \
	    --upload-file $(BIN)-static-$(VERSION).x86_64.tar.gz

clean:
	$(RM) -f $(BIN) nnn-$(VERSION).tar.gz *.sig $(BIN)-static $(BIN)-static-$(VERSION).x86_64.tar.gz

prepatch:
ifeq ($(strip $(O_NAMEFIRST)),1)
	patch --forward --strip=1 --input=$(NAMEFIRST)/mainline.diff
ifeq ($(strip $(O_GITSTATUS)),1)
	patch --forward --strip=1 --input=$(GITSTATUS)/namefirst.diff
endif
else ifeq ($(strip $(O_GITSTATUS)),1)
	patch --forward --strip=1 --input=$(GITSTATUS)/mainline.diff
endif

postpatch:
ifeq ($(strip $(O_NAMEFIRST)),1)
ifeq ($(strip $(O_GITSTATUS)),1)
	patch --reverse --strip=1 --input=$(GITSTATUS)/namefirst.diff
endif
	patch --reverse --strip=1 --input=$(NAMEFIRST)/mainline.diff
else ifeq ($(strip $(O_GITSTATUS)),1)
	patch --reverse --strip=1 --input=$(GITSTATUS)/mainline.diff
endif

skip: ;

.PHONY: all install uninstall strip static dist sign upload-local clean
