/*
	CHIP-8 Emulator
	Author: Crispin Corpuz
	Date: 12/23/2021

	Reference: https://tobiasvl.github.io/blog/write-a-chip-8-emulator/

	The purpose of this is to learn how to implement a CHIP-8 emulator/
	interpreter in ANSI C (if at all possible). Otherwise, C++ will be 
	used. 

	Additionally, the visuals may be handled with different libraries, 
	since ANSI C is not oriented toward that necessarily. (And to build 
	it would be time consuming and foolish)
	
*/

#include "main.h"
#include <errno.h>
#include <curses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <windows.h>

/*
 * 	Stack functions
 */
uint16_t stackIsFull(struct Stack* stack) {
	return stack->top == stack->capacity-1;
}

uint16_t stackIsEmpty(struct Stack* stack) {
	return stack->top == -1;
}

uint16_t pop(struct Stack* stack) {
	if (stackIsEmpty(stack)) 
	{
		printf("Stack is empty, nothing to pop\n");
		return -1;
	}
	printf("Popped 0x%x from stack\n", stack->array[stack->top]);
	return stack->array[stack->top--];
}

uint16_t push(struct Stack* stack, uint16_t value) {
	if (stackIsFull(stack))
	{
		printf("Stack is full, could not push %x\n",value);
		return -1;
	}
	stack->array[++stack->top] = value;
	printf("Pushed 0x%x to stack\n", stack->array[stack->top]);
	return stack->array[stack->top];
}

uint16_t peek(struct Stack* stack) {
	if (stackIsEmpty(stack))
		return -1;
	return stack->array[stack->top];
}

struct Stack* createStack(unsigned capacity) {
	struct Stack* stack = (struct Stack*)malloc(sizeof(struct Stack*));
    stack->capacity = capacity;
    stack->top = -1;
    stack->array = (uint16_t*)malloc(stack->capacity * sizeof(uint16_t));
    return stack;
}

void loadFont(uint8_t* memory) {
	uint8_t index = FONT_ADDRESS_L, count = 0;
	char ch;
	char* filename = "font.txt";
	FILE* fp = fopen(filename, "r");

	printf("Loading fonts from %s\n",filename);
	if (fp == NULL) 
	{
		printf("Could not open font file %s\n", filename);
	}
	while ((ch=fgetc(fp)) != EOF)
	{
		if (ch == 'x' || ch == ' ' || ch == '\n')
			continue;
		else
		{
			if (count++ % 2)
				memory[index++] += (ch&15)+(ch>>6)*9;
			else
				memory[index] = ((ch&15)+(ch>>6)*9)<<4;
		} 
	}
	fclose(fp);
	printf("Font loaded successfully\n");
}

uint8_t loadProgram(uint8_t* memory) {
	uint16_t start = PROGRAM_SPACE_START, end = RAMSIZE, i = 0, j = 0;
	char* filename = "roms/flightrunner.ch8";
	unsigned char* buffer = (unsigned char*) malloc(end-start);
	FILE* rom = fopen(filename, "rb");

	printf("Loading program from %s\n", filename);
	if (rom == NULL) 
		return -1;
	fread(buffer, 1, end-start, rom);
	for (i = PROGRAM_SPACE_START; i < RAMSIZE; i++) {
		memory[i] = buffer[j++];
	}
	printf("Loaded program\n");
	return 1;
}

uint8_t update(int frequency) {
	struct timeb t_start, t_current;
	int t_diff = 0;
	ftime(&t_start);
	do {
		ftime(&t_current);
		t_diff = (int) (1000.0 * (t_current.time - t_start.time) + (t_current.millitm - t_start.millitm));     
	} while(t_diff < 1000/frequency);
	return (uint8_t) t_diff;
}

void initializeScreen(int width, int height) {
	initscr();
	noecho();
	resize_term(DISPLAY_HEIGHT, DISPLAY_WIDTH);
	refresh();
	return;
}

/* Program execution functions */
void incrementPC(uint8_t* RAM) {
	if (RAM[PROGRAM_COUNTER_L] >= 0xfe)
	{
		RAM[PROGRAM_COUNTER_L] = 0;
		RAM[PROGRAM_COUNTER_H]++;
	}
	else
		RAM[PROGRAM_COUNTER_L] += 2;
	return;
}

uint16_t fetch(uint8_t* RAM) {	
	uint16_t instruction = (RAM[PROGRAM_COUNTER_H] << 8) & RAM[PROGRAM_COUNTER_L];
	incrementPC(RAM);
	return instruction;
}

uint16_t execute(uint8_t* RAM, struct Stack* stack, uint16_t opcode) {
	uint16_t first = (opcode & 0xf000) >> 12;
	uint16_t second = (opcode & 0x000f);
	uint16_t temp1 = 0, temp2 = 0;
	switch (first)
	{
		
	case 0: 
		switch (opcode)
		{
		case 0x00E0: /* clear screen */
			/* code */
			break;
		case 0x00EE: /* return from subroutine */
			RAM[PROGRAM_COUNTER_H] = pop(stack);
			RAM[PROGRAM_COUNTER_L] = pop(stack);
			break;
		
		default:
			break;
		}
		break;

	case 1: /* Jump to 0x0NNN */
		RAM[PROGRAM_COUNTER_H] = (opcode & 0x0f00) >> 8;
		RAM[PROGRAM_COUNTER_L] = (opcode & 0x00ff);
		break;

	case 2: /* go to subroutine at 0x0NNN */
		push(stack,RAM[PROGRAM_COUNTER_L]);
		push(stack,RAM[PROGRAM_COUNTER_H]);
		RAM[PROGRAM_COUNTER_H] = (opcode & 0x0f00) >> 8;
		RAM[PROGRAM_COUNTER_L] = (opcode & 0x00ff);
		break;

	case 3:
		if (RAM[(0x40 | ((opcode & 0x0f00) >> 8))] == (opcode & 0x00ff))
			incrementPC(RAM);
		break;

	case 4:
		if (RAM[(0x40 | ((opcode & 0x0f00) >> 8))] != (opcode & 0x00ff))
			incrementPC(RAM);
		break;

	case 5:
		if (RAM[(0x40 | ((opcode & 0x0f00) >> 8))] == RAM[(0x40 | ((opcode & 0x00f0) >> 4))])
			incrementPC(RAM);
		break;

	case 6:
		RAM[(0x40 | ((opcode & 0x0f00) >> 8))] = opcode & 0x00ff;
		break;

	case 7:
		RAM[(0x40 | ((opcode & 0x0f00) >> 8))] += opcode & 0x00ff;
		break;

	case 8:
		switch (second)
		{
		case 0: /* VX = VY; */
			RAM[(0x40 | ((opcode & 0x0f00) >> 8))] = RAM[(0x40 | ((opcode & 0x00f0) >> 8))];
			break;
		case 1: /* VX |= VY; */
			RAM[(0x40 | ((opcode & 0x0f00) >> 8))] |= RAM[(0x40 | ((opcode & 0x00f0) >> 8))];
			break;
		case 2: /* VX &= VY; */
			RAM[(0x40 | ((opcode & 0x0f00) >> 8))] &= RAM[(0x40 | ((opcode & 0x00f0) >> 8))];
			break;
		case 3: /* VX ^= VY; */
			RAM[(0x40 | ((opcode & 0x0f00) >> 8))] ^= RAM[(0x40 | ((opcode & 0x00f0) >> 8))];
			break;
		case 4: /* VX += VY; */
			temp1 = RAM[(0x40 | ((opcode & 0x0f00) >> 8))]; /* VX */
			temp2 = RAM[(0x40 | ((opcode & 0x00f0) >> 8))]; /* VY */
			if ((0xff - temp1) < temp2)
			{
				RAM[(0x40 | ((opcode & 0x0f00) >> 8))] += temp2;
				RAM[VF] = 1;
			}
			else
			{
				RAM[(0x40 | ((opcode & 0x0f00) >> 8))] += temp2;
				RAM[VF] = 0;
			}
			break;

		case 5: /* VX -= VY; */
			temp1 = RAM[(0x40 | ((opcode & 0x0f00) >> 8))]; /* VX */
			temp2 = RAM[(0x40 | ((opcode & 0x00f0) >> 8))]; /* VY */
			if (temp1 > temp2)
			{
				RAM[(0x40 | ((opcode & 0x0f00) >> 8))] -= temp2;
				RAM[VF] = 1;
			}
			else
			{
				RAM[(0x40 | ((opcode & 0x0f00) >> 8))] -= temp2;
				RAM[VF] = 0;
			}
			break;
		case 6: /* VX >> 1 in original COSMAC VIP, in-place shift in CHIP-48, SUPER-CHIP */
			RAM[(0x40 | ((opcode & 0x0f00) >> 8))] = RAM[(0x40 | ((opcode & 0x00f0) >> 8))] >> 1;
			RAM[VF] = RAM[(0x40 | ((opcode & 0x00f0) >> 8))] & 0x01; 
			break;
		case 7: /* VX = VY - VX; */
			temp1 = RAM[(0x40 | ((opcode & 0x0f00) >> 8))]; /* VX */
			temp2 = RAM[(0x40 | ((opcode & 0x00f0) >> 8))]; /* VY */
			if (temp2 < temp1)
			{
				RAM[(0x40 | ((opcode & 0x0f00) >> 8))] = temp2 - temp1;
				RAM[VF] = 1;
			}
			else
			{
				RAM[(0x40 | ((opcode & 0x0f00) >> 8))] = temp2 - temp1;
				RAM[VF] = 0;
			}
			break;
		case 0xE:
			RAM[(0x40 | ((opcode & 0x0f00) >> 8))] = RAM[(0x40 | ((opcode & 0x00f0) >> 8))] << 1;
			RAM[VF] = RAM[(0x40 | ((opcode & 0x00f0) >> 8))] & 0x20; 
			break;
		default:
			/* TODO: Put a debug print here to emulator console */
			break;
		}
		break;

	case 9:
		if (RAM[(0x40 | ((opcode & 0x0f00) >> 8))] != RAM[(0x40 | ((opcode & 0x00f0) >> 4))])
			incrementPC(RAM);
		break;

	case 0xA:
		RAM[I_REG_H] = (opcode & 0x0f00) >> 8;
		RAM[I_REG_L] = opcode & 0x00ff;
		break;

	case 0xB:
		temp1 = (opcode & 0x0fff) + RAM[V0];
		RAM[PROGRAM_COUNTER_H] = (opcode & 0x0f00) >> 8;
		RAM[PROGRAM_COUNTER_L] = opcode & 0x00ff;
		break;

	case 0xC: /* Random */ 
		RAM[(0x40 | ((opcode & 0x0f00) >> 8))] = (uint8_t) rand() & (uint8_t) (opcode & 0x00ff);
		break;

	case 0xD:

		break;

	case 0xE:

		break;

	case 0xF:

		break;

	default:
		/* TODO: Put a debug print here to emulator console */
		break;
	}
	return 1;
}

int main() {
	uint16_t opcode = 0;
	uint8_t* RAM;
	int i; /* general memory FOR loop iterator */
	struct Stack* stack;

	/* Initializing memory */
	RAM = malloc(RAMSIZE * sizeof(uint8_t));
	stack = createStack(STACK_TOP);
	for (i = 0x0; i < 0x200; i++) { 
		RAM[i] = 0;
	}

	/* Loading font from file */
	loadFont(RAM);

	/* Loading CHIP-8 Program */
	loadProgram(RAM);
	RAM[PROGRAM_COUNTER_H] = 0x02;
	RAM[PROGRAM_COUNTER_L] = 0x00;

	/* Initialize screens */
	initializeScreen(64, 32);
	for (i = 0; i < 0x126; i++) {
		opcode = fetch(RAM);
		execute(RAM, stack, opcode);
	}
	getch();
	endwin();
	
	/* Memory after loading all data */
	printf("********** MEMORY **********\n");
	for (i = 0x000; i < 0x330; i++) {
		printf("0x%x\t: 0x%x\n",i,RAM[i]);
	}
	printf("******** END MEMORY ********\n");

	/* Machine Initialization */
	

	/* Decoding the program data (opcodes) */

	return 0;
} /* end main() */
