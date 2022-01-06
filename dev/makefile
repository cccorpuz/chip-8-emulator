CFLAGS := -Wall -ansi -pedantic -c -g 

ALL: main

main : main.o
	gcc -g -o chip8.exe main.o -lpdcurses

main.o : main.c 
	gcc $(CFLAGS) main.c -lpdcurses

clean: 
	rm -f *.o
