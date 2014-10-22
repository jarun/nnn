PREFIX = /usr/local
MANPREFIX = $(PREFIX)/man

#CPPFLAGS += -DDEBUG
#CFLAGS += -g
LDLIBS = -lncursesw

OBJ = noice.o strlcpy.o
BIN = noice

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

noice.o: queue.h util.h config.h
strlcpy.o: util.h

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN).1 $(DESTDIR)$(MANPREFIX)/man1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)
	rm -f $(DESTDIR)$(MANPREFIX)/man1/$(BIN).1

clean:
	rm -f $(BIN) $(OBJ)

.SUFFIXES: .def.h

.def.h.h:
	cp $< $@
