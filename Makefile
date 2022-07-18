OBJS=main.o
BIN=chip8
CFLAGS=-Wall -Wextra -pedantic
LFLAGS=-lSDL2

all : main.o
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS) $(LFLAGS)

clean :
	rm $(BIN) $(OBJS)
