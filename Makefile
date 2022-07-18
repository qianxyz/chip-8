CFLAGS=-Wall -Wextra -pedantic

all : main.o
	$(CC) $(CFLAGS) -o chip8 main.o

clean :
	rm chip8 main.o
