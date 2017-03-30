VERSION = 0.6

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/man

#CPPFLAGS = -DDEBUG
#CFLAGS = -g
CFLAGS = -O3 -march=native
LDLIBS = -lcurses

DISTFILES = nnn.c strlcat.c strlcpy.c util.h config.def.h\
    nnn.1 Makefile README.md LICENSE
OBJ = nnn.o strlcat.o strlcpy.o
BIN = nnn

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS) $(LDLIBS)
	strip $(BIN)

nnn.o: util.h config.h
strlcat.o: util.h
strlcpy.o: util.h

config.h:
	cp config.def.h $@

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
	rm -f config.h $(BIN) $(OBJ) nnn-$(VERSION).tar.gz
