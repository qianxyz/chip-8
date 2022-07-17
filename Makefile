all : main.o
	cc -o chip8 main.o

clean :
	rm chip8 main.o
