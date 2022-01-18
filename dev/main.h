#include <stdint.h>

#define DISPLAY_HEIGHT 32
#define DISPLAY_WIDTH 64
#define RAMSIZE 4096
#define STACK_BOTTOM 0
#define STACK_TOP 16

#define DELAY_TIMER 0x3a
#define SOUND_TIMER 0x3b

#define I_REG_H 0x3c
#define I_REG_L 0x3d

#define PROGRAM_COUNTER_H 0x3e
#define PROGRAM_COUNTER_L 0x3f

/* General Purpose Registers */
#define V0 0x40
#define V1 0x41
#define V2 0x42
#define V3 0x43
#define V4 0x44
#define V5 0x45
#define V6 0x46
#define V7 0x47
#define V8 0x48
#define V9 0x49
#define VA 0x4a
#define VB 0x4b
#define VC 0x4c
#define VD 0x4d
#define VE 0x4e
#define VF 0x4f

#define FONT_ADDRESS_L 0x50
#define FONT_ADDRESS_H 0x9f

#define PROGRAM_SPACE_START 0x200

struct Stack {
	uint16_t top;
	unsigned capacity;
	uint16_t* array;
};

uint16_t stackIsFull(struct Stack* stack);
uint16_t stackIsEmpty(struct Stack* stack);
uint16_t pop(struct Stack* stack);
uint16_t push(struct Stack* stack, uint16_t value);
uint16_t peek(struct Stack* stack);
struct Stack* createStack(unsigned capacity);
void loadFont(uint8_t* memory);
uint8_t loadProgram(uint8_t* memory);
uint8_t update(int frequency);
void initializeScreen(int width, int height);

/* Program execution functions */
void incrementPC(uint8_t* RAM);
uint16_t fetch(uint8_t* RAM);
uint16_t execute(uint8_t* ram, struct Stack* stack, uint16_t opcode);

/* Initialize SDL */
int initializeSDL();
