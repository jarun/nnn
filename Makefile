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
	@echo installing $(BIN) to $(DESTDIR)$(PREFIX)/bin
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	@chmod 755 $(DESTDIR)$(PREFIX)/bin/$(BIN)
	@echo installing $(BIN).1 to $(DESTDIR)$(MANPREFIX)/man1
	@cp -f $(BIN).1 $(DESTDIR)$(MANPREFIX)/man1

uninstall:
	@echo removing $(BIN) from $(DESTDIR)$(PREFIX)/bin
	@rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)
	@echo removing $(BIN).1 from $(DESTDIR)$(MANPREFIX)/man1
	@rm -f $(DESTDIR)$(MANPREFIX)/man1/$(BIN).1

clean:
	rm -f $(BIN) $(OBJ)

.SUFFIXES: .def.h

.def.h.h:
	cp $< $@
