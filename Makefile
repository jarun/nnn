#CPPFLAGS += -DDEBUG
#CFLAGS += -g
LDLIBS = -lncursesw
OBJ = noice.o strlcpy.o
BIN = noice

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDLIBS)

noice.o: noice.c queue.h util.h
	$(CC) -c noice.c

strlcpy.o: strlcpy.c util.h
	$(CC) -c strlcpy.c

clean:
	rm -f $(BIN) $(OBJ)
