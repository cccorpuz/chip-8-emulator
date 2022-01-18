CFLAGS := -Wall -ansi -pedantic -c -g 
SDLFLAGS := -IC:\Users\14025\OneDrive\Desktop\chip-8-emulator\dev\SDL2\include -LC:\Users\14025\OneDrive\Desktop\chip-8-emulator\dev\SDL2\lib -lmingw32 -lSDL2main -lSDL2 

ALL: main

main : main.o
	gcc -g -o chip8.exe main.o

main.o : main.c 
	gcc -g main.c -o $(CFLAGS)  

clean: 
	rm -f *.o
