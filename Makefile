#CPPFLAGS += -DDEBUG
#CFLAGS += -g
LDLIBS = -lncursesw
BIN = noice

all: $(BIN)

clean:
	rm -f $(BIN)
