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
O_NOLC := 0  # no locale support
O_NOMOUSE := 0  # no mouse support
O_NOBATCH := 0  # no built-in batch renamer
O_NOFIFO := 0  # no FIFO previewer support
O_CTX8 := 0  # enable 8 contexts
O_ICONS := 0  # support icons-in-terminal
O_NERD := 0  # support icons-nerdfont
O_EMOJI := 0  # support emoji
O_QSORT := 0  # use Alexey Tourbin's QSORT implementation
O_BENCH := 0  # benchmark mode (stops at first user input)
O_NOSSN := 0  # disable session support
O_NOUG := 0  # disable user, group name in status bar
O_NOX11 := 0  # disable X11 integration
O_MATCHFLTR := 0  # allow filters without matches
O_NOSORT := 0  # disable sorting entries on dir load

# User patches
O_COLEMAK := 0 # change key bindings to colemak compatible layout
O_COLEMAK-DH := 0 # change key bindings to colemak-dh compatible layout
O_GITSTATUS := 0 # add git status to detail view
O_NAMEFIRST := 0 # print file name first, add uid and guid to detail view
O_RESTOREPREVIEW := 0 # add preview pipe to close and restore preview pane

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
	CFLAGS += -g3
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
	else ifeq ($(strip $(O_EMOJI)),1)
$(info *** Ignoring O_NOLC since O_EMOJI is set ***)
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
	ICONS_INCLUDE = icons-generated-icons-in-term.h
	CPPFLAGS += -DICONS_IN_TERM -DICONS_INCLUDE=\"$(ICONS_INCLUDE)\"
endif

ifeq ($(strip $(O_NERD)),1)
	ICONS_INCLUDE = icons-generated-nerd.h
	CPPFLAGS += -DNERD -DICONS_INCLUDE=\"$(ICONS_INCLUDE)\"
endif

ifeq ($(strip $(O_EMOJI)),1)
	ICONS_INCLUDE = icons-generated-emoji.h
	CPPFLAGS += -DEMOJI -DICONS_INCLUDE=\"$(ICONS_INCLUDE)\"
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

ifeq ($(strip $(O_NOSORT)),1)
	CPPFLAGS += -DNOSORT
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

LDLIBS += $(LDLIBS_CURSES) -lpthread

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

COLEMAK = patches/colemak
COLEMAK-DH = patches/colemak-dh
GITSTATUS = patches/gitstatus
NAMEFIRST = patches/namefirst
RESTOREPREVIEW = patches/restorepreview

# test if we are on Mac OS X and get X.Y.Z OS version with system binary /usr/bin/sw_vers
MACOS_VERSION := $(strip $(shell command -v sw_vers >/dev/null && [ "`sw_vers -productName`" = "Mac OS X" ] && sw_vers -productVersion))
# if Mac OS X detected, test if its version is below 10.12.0 relying on "sort -c" returning "disorder" message if the input is not sorted
ifneq ($(MACOS_VERSION),)
	MACOS_BELOW_1012 := $(if $(strip $(shell printf '10.12.0\n%s' "$(MACOS_VERSION)" | sort -ct. -k1,1n -k2,2n -k3,3n 2>&1)),1)
endif
# if Mac OS X version is below 10.12.0, compile in the replacement clock_gettime and define MACOS_BELOW_1012 so that it's included in nnn.c
ifneq ($(MACOS_BELOW_1012),)
	GETTIME_C = misc/macos-legacy/mach_gettime.c
	GETTIME_H = misc/macos-legacy/mach_gettime.h
	SRC += $(GETTIME_C)
	HEADERS += $(GETTIME_H)
	CPPFLAGS += -DMACOS_BELOW_1012
endif

ifeq ($(strip $(O_DEBUG)),1)
	HEADERS += src/dbg.h
endif
ifeq ($(strip $(O_QSORT)),1)
	HEADERS += src/qsort.h
endif
ifeq ($(strip $(O_EMOJI)),1)
	HEADERS += src/icons.h src/$(ICONS_INCLUDE)
endif
ifeq ($(strip $(O_NERD)),1)
	HEADERS += src/icons.h src/$(ICONS_INCLUDE)
endif
ifeq ($(strip $(O_ICONS)),1)
	HEADERS += src/icons.h src/$(ICONS_INCLUDE) src/icons-in-terminal.h
endif

all: $(BIN)

$(BIN): $(SRC) $(HEADERS) Makefile
	@$(MAKE) --silent prepatch
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $(GETTIME_C) $< $(LDLIBS)
	@$(MAKE) --silent postpatch

# targets for backwards compatibility
debug: $(BIN)
norl: $(BIN)
nolc: $(BIN)

src/$(ICONS_INCLUDE): src/icons-hash.c src/icons.h src/icons-in-terminal.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -DICONS_GENERATE -o src/icons-hash-gen src/icons-hash.c
	./src/icons-hash-gen > $@

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
	# static binary with emoji support
	make O_STATIC=1 O_EMOJI=1 strip
	mv $(BIN) $(BIN)-emoji-static

musl:
	cp misc/musl/musl-static-ubuntu.sh .
	./musl-static-ubuntu.sh 1
	rm ./musl-static-ubuntu.sh

shellcheck:
	find ./plugins/ -type f -not -name "*.md" -exec shellcheck {} +

dist:
	mkdir -p nnn-$(VERSION)
	$(CP) -r $(DISTFILES) nnn-$(VERSION)
	tar -cf - nnn-$(VERSION) | gzip > nnn-$(VERSION).tar.gz
	$(RM) -r nnn-$(VERSION)

sign:
	git archive -o nnn-$(VERSION).tar.gz --format tar.gz --prefix=nnn-$(VERSION)/ v$(VERSION)
	gpg --detach-sign --yes nnn-$(VERSION).tar.gz
	rm -f nnn-$(VERSION).tar.gz

upload-local: sign static musl
	$(eval ID=$(shell curl -s 'https://api.github.com/repos/jarun/nnn/releases/tags/v$(VERSION)' | jq .id))
	# upload sign file
	curl -XPOST 'https://uploads.github.com/repos/jarun/nnn/releases/$(ID)/assets?name=nnn-$(VERSION).tar.gz.sig' \
	    -H 'Authorization: token $(NNN_SIG_UPLOAD_TOKEN)' -H 'Content-Type: application/pgp-signature' \
	    --upload-file nnn-$(VERSION).tar.gz.sig
	# upx compress all static binaries
	upx -qqq $(BIN)-static
	upx -qqq $(BIN)-icons-static
	upx -qqq $(BIN)-nerd-static
	upx -qqq $(BIN)-emoji-static
	# upload static binary
	tar -zcf $(BIN)-static-$(VERSION).x86_64.tar.gz $(BIN)-static
	curl -XPOST 'https://uploads.github.com/repos/jarun/nnn/releases/$(ID)/assets?name=$(BIN)-static-$(VERSION).x86_64.tar.gz' \
	    -H 'Authorization: token $(NNN_SIG_UPLOAD_TOKEN)' -H 'Content-Type: application/x-sharedlib' \
	    --upload-file $(BIN)-static-$(VERSION).x86_64.tar.gz
	# upload icons-in-terminal compiled static binary
	tar -zcf $(BIN)-icons-static-$(VERSION).x86_64.tar.gz $(BIN)-icons-static
	curl -XPOST 'https://uploads.github.com/repos/jarun/nnn/releases/$(ID)/assets?name=$(BIN)-icons-static-$(VERSION).x86_64.tar.gz' \
	    -H 'Authorization: token $(NNN_SIG_UPLOAD_TOKEN)' -H 'Content-Type: application/x-sharedlib' \
	    --upload-file $(BIN)-icons-static-$(VERSION).x86_64.tar.gz
	# upload patched nerd font compiled static binary
	tar -zcf $(BIN)-nerd-static-$(VERSION).x86_64.tar.gz $(BIN)-nerd-static
	curl -XPOST 'https://uploads.github.com/repos/jarun/nnn/releases/$(ID)/assets?name=$(BIN)-nerd-static-$(VERSION).x86_64.tar.gz' \
	    -H 'Authorization: token $(NNN_SIG_UPLOAD_TOKEN)' -H 'Content-Type: application/x-sharedlib' \
	    --upload-file $(BIN)-nerd-static-$(VERSION).x86_64.tar.gz
	# upload emoji compiled static binary
	tar -zcf $(BIN)-emoji-static-$(VERSION).x86_64.tar.gz $(BIN)-emoji-static
	curl -XPOST 'https://uploads.github.com/repos/jarun/nnn/releases/$(ID)/assets?name=$(BIN)-emoji-static-$(VERSION).x86_64.tar.gz' \
	    -H 'Authorization: token $(NNN_SIG_UPLOAD_TOKEN)' -H 'Content-Type: application/x-sharedlib' \
	    --upload-file $(BIN)-emoji-static-$(VERSION).x86_64.tar.gz
	# upload musl static binary
	tar -zcf $(BIN)-musl-static-$(VERSION).x86_64.tar.gz $(BIN)-musl-static
	curl -XPOST 'https://uploads.github.com/repos/jarun/nnn/releases/$(ID)/assets?name=$(BIN)-musl-static-$(VERSION).x86_64.tar.gz' \
	    -H 'Authorization: token $(NNN_SIG_UPLOAD_TOKEN)' -H 'Content-Type: application/x-sharedlib' \
	    --upload-file $(BIN)-musl-static-$(VERSION).x86_64.tar.gz

clean:
	$(RM) -f $(BIN) nnn-$(VERSION).tar.gz *.sig $(BIN)-static $(BIN)-static-$(VERSION).x86_64.tar.gz $(BIN)-icons-static $(BIN)-icons-static-$(VERSION).x86_64.tar.gz $(BIN)-nerd-static $(BIN)-nerd-static-$(VERSION).x86_64.tar.gz $(BIN)-emoji-static $(BIN)-emoji-static-$(VERSION).x86_64.tar.gz $(BIN)-musl-static $(BIN)-musl-static-$(VERSION).x86_64.tar.gz src/icons-hash-gen src/icons-generated-*.h

checkpatches:
	./patches/check-patches.sh

prepatch:
ifeq ($(strip $(O_NAMEFIRST)),1)
	patch --forward $(PATCH_OPTS) --strip=1 --input=$(NAMEFIRST)/mainline.diff
ifeq ($(strip $(O_GITSTATUS)),1)
	patch --forward $(PATCH_OPTS) --strip=1 --input=$(GITSTATUS)/namefirst.diff
endif
else ifeq ($(strip $(O_GITSTATUS)),1)
	patch --forward $(PATCH_OPTS) --strip=1 --input=$(GITSTATUS)/mainline.diff
endif
ifeq ($(strip $(O_RESTOREPREVIEW)),1)
	patch --forward $(PATCH_OPTS) --strip=1 --input=$(RESTOREPREVIEW)/mainline.diff
endif
ifeq ($(strip $(O_COLEMAK)),1)
	patch --forward $(PATCH_OPTS) --strip=1 --input=$(COLEMAK)/mainline.diff
endif
ifeq ($(strip $(O_COLEMAK-DH)),1)
	patch --forward $(PATCH_OPTS) --strip=1 --input=$(COLEMAK-DH)/mainline.diff
endif

postpatch:
ifeq ($(strip $(O_NAMEFIRST)),1)
ifeq ($(strip $(O_GITSTATUS)),1)
	patch --reverse $(PATCH_OPTS) --strip=1 --input=$(GITSTATUS)/namefirst.diff
endif
	patch --reverse $(PATCH_OPTS) --strip=1 --input=$(NAMEFIRST)/mainline.diff
else ifeq ($(strip $(O_GITSTATUS)),1)
	patch --reverse $(PATCH_OPTS) --strip=1 --input=$(GITSTATUS)/mainline.diff
endif
ifeq ($(strip $(O_RESTOREPREVIEW)),1)
	patch --reverse $(PATCH_OPTS) --strip=1 --input=$(RESTOREPREVIEW)/mainline.diff
endif
ifeq ($(strip $(O_COLEMAK)),1)
	patch --reverse $(PATCH_OPTS) --strip=1 --input=$(COLEMAK)/mainline.diff
endif
ifeq ($(strip $(O_COLEMAK-DH)),1)
	patch --reverse $(PATCH_OPTS) --strip=1 --input=$(COLEMAK-DH)/mainline.diff
endif

skip: ;

.PHONY: all install uninstall strip static dist sign upload-local clean install-desktop uninstall-desktop
