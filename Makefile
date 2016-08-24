VERSION = 0.6

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/man

#CPPFLAGS = -DDEBUG
#CFLAGS = -g
LDLIBS = -lcurses

DISTFILES = noice.c strlcat.c strlcpy.c util.h config.def.h\
    noice.1 Makefile README LICENSE
OBJ = noice.o strlcat.o strlcpy.o
BIN = noice

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS) $(LDLIBS)

noice.o: util.h config.h
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
	mkdir -p noice-$(VERSION)
	cp $(DISTFILES) noice-$(VERSION)
	tar -cf noice-$(VERSION).tar noice-$(VERSION)
	gzip noice-$(VERSION).tar
	rm -rf noice-$(VERSION)

clean:
	rm -f $(BIN) $(OBJ) noice-$(VERSION).tar.gz
