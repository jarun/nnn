#CPPFLAGS += -DDEBUG
#CFLAGS += -g
LDLIBS = -lncursesw
#LDLIBS += -lbsd
BIN = noice

all: $(BIN)

clean:
	rm -f $(BIN)
