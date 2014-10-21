#CPPFLAGS += -DDEBUG
#CFLAGS += -g
LDLIBS = -lncursesw
OBJ = noice.o strlcpy.o
BIN = noice

all: $(BIN)

$(BIN): config.h $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

config.h:
	@echo copying config.def.h to $@
	@cp config.def.h $@

noice.o: noice.c queue.h util.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c noice.c

strlcpy.o: strlcpy.c util.h
	$(CC) $(CFLAGS) -c strlcpy.c

clean:
	rm -f $(BIN) $(OBJ)
