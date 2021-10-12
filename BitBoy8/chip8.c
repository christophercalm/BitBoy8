#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>      
#include "chip8.h"

//todo encapsulate into a struct
uint8_t V[NUM_REGISTERS] = { 0 };
uint8_t memory[SIZE_MEMORY] = { 0 };
uint16_t idx = 0;
uint16_t pc = 0;
uint16_t stack[STACK_LEVELS] = { 0 };
uint16_t sp = 0;
uint8_t delay_timer = 0;
uint8_t sound_timer = 0;
uint8_t keypad[KEYPAD_SIZE] = { 0 };
uint8_t video[SCREEN_WIDTH * SCREEN_HEIGHT] = { 0 };
uint8_t draw_flag = 0;
uint16_t opcode = 0;

uint8_t debug_flag = 1;

uint8_t x;
uint8_t y;
uint8_t n;
uint8_t nn;
uint16_t nnn;

uint8_t fontset[FONTSET_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// TODO : make this more uniform
uint8_t get_random_byte() {
	return rand() % 0xff;
}


//CLS
void op_00EO() {
	memset(video, 0, sizeof(video));
	draw_flag = 1;
}

//RET
void op_00EE() {
	--sp;
	pc = stack[sp];
}

//JP addr
void op_1NNN() {
	pc = nnn;
}

// call addr
void op_2NNN() {
	stack[sp] = pc;
	++sp;
	pc = nnn;
}

//SE Vx, byte
void op_3XNN() {
	if (V[x] == nn) {
		pc += 2;
	}
}

//SNE Vx, byte
void op_4XNN() {
	if (V[x] != nn) {
		pc += 2;
	}
}

//SE Vx, Vy
void op_5XY0() {
	if (V[x] == V[y]) {
		pc += 2;
	}
}

//LD Vx, byte
void op_6XNN() {
	V[x] = nn;
}

//ADD Vx, byte
void op_7XNN() {
	V[x] += nn;
}

//LD Vx, byte
void op_8XY0() {
	V[x] = V[y];
}

//OR Vx, Vy
void op_8XY1() {
	V[x] = V[x] | V[y];
}

//AND Vx, Vy
void op_8XY2() {
	V[x] = V[x] & V[y];
}

//XOR Vx, Vy
void op_8XY3() {
	V[x] = V[x] ^ V[y];
}

//ADD Vx, Vy
void op_8XY4() {
	if (V[x] + V[y] > 255) {
		V[0xF] = 1;
	}
	else {
		V[0xF] = 0;
	}

	V[x] = V[x] + V[y];
}

//SUB Vx, Vy
void op_8XY5() {
	if (V[x] > V[y]) {
		V[0xF] = 1;
	}
	else {
		V[0xF] = 0;
	}

	V[x] = V[x] - V[y];
}

//SHR Vx {, Vy
//If the least-significant bit of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2.
void op_8XY6() {
	V[0xF] = (V[x] & 0x1u);

	V[x] >>= 1;
}

//SUBN Vx, Vy
void op_8XY7() {
	if (V[y] > V[x])
	{
		V[0xF] = 1;
	}
	else
	{
		V[0xF] = 0;
	}

	V[x] = V[y] - V[x];
	/*if (V[x] > V[y]) {
		V[0xF] = 1;
	}
	else {
		V[0xF] = 0;
	}

	V[x] = V[y] - V[x];*/
}

//SHL Vx {, Vy}
void op_8XYE() {
	if (get_bit_from_byte(V[x], 7)) {
		V[0xF] = 1;
	}
	else {
		V[0xF] = 0;
	}

	V[x] <<= 1;
}

//SNE Vx, Vy
void op_9XY0() {
	if (V[x] != V[y]) {
		pc += 2;
	}
}

void op_ANNN() {
	idx = nnn;
}

// JMP with offset
void op_BNNN() {
	//this instruction jumped to the address NNN plus the value in the register V0.
	pc = nnn + V[0];
}

//CXNN: Random
void op_CXNN() {
	V[x] = get_random_byte() & nn;
}



/**
 *  It will draw an N pixels tall sprite from the memory location that the I index register is holding to the screen,
 *  at the horizontal X coordinate in VX and the Y coordinate in VY. All the pixels that are “on” in the sprite will flip the pixels on the screen that it is drawn to.
 *  If any pixels on the screen were turned “off” by this, the VF flag register is set to 1. Otherwise, it’s set to 0.
 **/
void op_DXYN() {
	draw_flag = 1;
	uint8_t x_pos = V[x] % SCREEN_WIDTH;
	uint8_t y_pos = V[y] % SCREEN_HEIGHT;
	V[0xF] = 0;
	for (uint8_t row = 0; row < n; row++) {
		uint8_t display_byte = memory[idx + row];
		for (uint8_t sprite_pixel = 0; sprite_pixel < WIDTH_SPRITE; sprite_pixel++) {
			if (get_bit_from_byte(display_byte, WIDTH_SPRITE - sprite_pixel - 1) && get_pixel_set_in_display(x_pos, y_pos)) {
				uint16_t index = get_pixel_array_index_in_display(x_pos, y_pos);
				printf("%d", index);
				video[index] = 0;
				V[0xF] = 1;
			}	else if (get_bit_from_byte(display_byte, WIDTH_SPRITE - sprite_pixel - 1) && !get_pixel_set_in_display(x_pos, y_pos)) {
				uint16_t index = get_pixel_array_index_in_display(x_pos, y_pos);
				printf("%d", index);
				video[index] = 1;
			}
			x_pos++;
		}
		x_pos = V[x] % SCREEN_WIDTH; //reset x position
		y_pos++;
		if (y_pos >= SCREEN_HEIGHT) {
			return;
		}
	}
}

//Skip if key
void op_EX9E() {
	if (keypad[V[x]] == 1) {
		pc += 2;
	}
}

//skip if not key
void op_EXA1() {
	if (keypad[V[x]] != 1) {
		pc += 2;
	}
}

//FX07, FX15 and FX18: Timers
void op_FX07() {
	V[x] = delay_timer;
}

void op_FX15() {
	delay_timer = V[x];
}

void op_FX18() {
	sound_timer = V[x];
}

//Add to index
void op_FX1E() {
	if (idx + V[x] > 0x0FFF) {
		V[0xf] = 1;
	}
	idx += V[x];
}

//wait for key
void op_FX0A() {
	uint8_t found = 0;
	for (uint8_t i = 0; i < KEYPAD_SIZE; i++) {
		if (keypad[i] == 1) {
			V[x] = i;
			found = 1;
		}
	}
	if (!found) {
		pc -= 2;
	}
}

// set index to character
void op_FX29() {
	uint8_t font_character = V[x];
	idx = FONTSET_START_ADDRESS + (font_character * KEY_BYTE_LENGTH);
}

// Binary-coded decimal conversion
void op_FX33() {
	//get number from V[x]
	uint8_t number = V[x];
	uint8_t hundreds = number / 100;
	uint8_t tens = (number - 100 * hundreds) / 10;
	uint8_t ones = (number - 100 * hundreds) % 10;

	//store bytes starting at index register 
	memory[idx] = hundreds;
	memory[idx + 1] = tens;
	memory[idx + 2] = ones;
}

// Store registers into memory up until register x from idx
void op_FX55() {
	for (uint8_t i = 0; i <= x; i++) {
		memory[idx + i] = V[i];
	}
}

// store memory into registers until register x from idx
void op_FX65() {
	for (uint8_t i = 0; i <= x; i++) {
		V[i] = memory[idx + i];
	}
}



void emulate_cycle() {

	// Fetch:
	opcode = (memory[pc] << 8) | memory[pc + 1];

	// Decode:
	// X: The second nibble. Used to look up one of the 16 registers (VX) from V0 through VF.
	// Y: The third nibble. Also used to look up one of the 16 registers (VY) from V0 through VF.
	// N: The fourth nibble. A 4-bit number.
	// NN: The second byte (third and fourth nibbles). An 8-bit immediate number.
	// NNN: The second, third and fourth nibbles. A 12-bit immediate memory address.
	x = memory[pc] & 0x0F;
	y = memory[pc + 1] >> 4;
	n = memory[pc + 1] & 0x0F;
	nn = memory[pc + 1];
	nnn = opcode & 0x0FFF;

	printf("%04x", opcode);


	//printf("%04x", opcode);

	//execute
	pc += 2;

	uint8_t unknown_instruction_hit = 0;

	//Mask off (with a binary AND) the first number in the instruction, and have one case per numbe
	uint16_t masked_opcode = (opcode & 0xF000) >> 12;

	

	switch (masked_opcode) {
	case 0x0:
		/* code */
		switch (opcode) {
		case 0x00E0:
			op_00EO();
			break;
		case 0x00EE:
			op_00EE();
			break;
		default:
			unknown_instruction_hit = 1;
			break;
		}
		break;
	case 0x1:
		op_1NNN();
		break;
	case 0x2:
		op_2NNN();
		break;
	case 0x3:
		op_3XNN();
		break;
	case 0x4:
		op_4XNN();
		break;
	case 0x5:
		op_5XY0();
		break;
	case 0x6:
		op_6XNN();
		break;
	case 0x7:
		op_7XNN();
		break;
	case 0x8:
		//switch on last nibble
		switch (n) { 
			case 0x0:
				op_8XY0();
				break;
			case 0x1:
				op_8XY1();
				break;
			case 0x2:
				op_8XY2();
				break;
			case 0x3:
				op_8XY3();
				break;
			case 0x4:
				op_8XY4();
				break;
			case 0x5:
				op_8XY5();
				break;
			case 0x6:
				op_8XY6();
				break;
			case 0x7:
				op_8XY7();
				break;
			case 0xE:
				op_8XYE();
				break;
			default:
				unknown_instruction_hit = 1;
				break;
		}
		break;
	case 0x9:
		op_9XY0();
		break;
	case 0xA:
		op_ANNN();
		break;
	case 0xB:
		op_BNNN();
		break;
	case 0xC:
		op_CXNN();
		break;
	case 0xD:
		op_DXYN();
		break;
	case 0xE:
		switch (n)
		{
		case 0xE:
			op_EX9E();
			break;
		case 0X1:
			op_EXA1();
			break;
		default:
			unknown_instruction_hit = 1;
			break;
		}
		break;
	case 0xF:
		switch (nn)
		{
		case 0x07:
			op_FX07();
			break;
		case 0x15:
			op_FX15();
			break;
		case 0x18:
			op_FX18();
			break;
		case 0x1E:
			op_FX1E();
			break;
		case 0x0A:
			op_FX0A();
			break;
		case 0x29:
			op_FX29();
			break;
		case 0x33:
			op_FX33();
			break;
		case 0x55:
			op_FX55();
			break;
		case 0x65:
			op_FX65();
			break;
		default:
			//unknown_instruction_hit = 1;
			break;
		}
		break;
	default:
		//unknown_instruction_hit = 1;
		break;
	}

	//decrement delay timer
	if (delay_timer) {
		--delay_timer;
	}

	//decrement sound timer
	if (sound_timer) {
		--sound_timer;
	}

	if (debug_flag == 1) {
		print_debugging_information();
		if (unknown_instruction_hit == 1) {
			printf("Unknown instruction: %04x\n", opcode);
		}

	}
}

void load_font() {
	for (uint8_t i = 0; i < FONTSET_SIZE; ++i) {
		memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}
}

uint8_t get_bit_from_byte(uint8_t byte, uint8_t bit) {
	bit = 1 << bit;
	return(bit & byte);
}	

uint8_t get_pixel_set_in_display(uint8_t xpos, uint8_t ypos) {
	uint16_t display_index = get_pixel_array_index_in_display(xpos, ypos);
	return video[display_index] == 1;
}

uint16_t get_pixel_array_index_in_display(uint8_t xpos, uint8_t ypos) {
	uint16_t index = SCREEN_WIDTH * ypos + xpos;
	if (index >= (SCREEN_HEIGHT * SCREEN_WIDTH)) {
		index = index % SCREEN_WIDTH;
		return index;
	}
	else {
		return index;
	}
}

void init_chip8() {
	pc = ROM_START;
	load_font();
	srand(time(NULL));
}

void print_debugging_information() {
	printf("*******************************\n");
	printf("8 bit Registers:\n");
	for (uint8_t i = 0; i <= 0xF; i++) {
		printf(" V%02x: %02x ", i, V[i]);
		if ((i + 1) % 8 == 0) {
			printf("\n");
		}
	}
	printf("16 bit registers:\n");
	printf(" idx: %04x pc: %04x sp: %04x op: %04x\n", idx, pc, sp, opcode);
	printf("Opcode Helpers: \n");
	printf("X: %02x Y: %02x N: %02x NN: %02x: NNN: %04x\n", x, y, n, nn, nnn);
	printf("Keypad:\n");
	for (uint8_t i = 0; i < KEYPAD_SIZE; i++) {
		printf(" Key%02x: %02x ", i, keypad[i]);
		if ((i + 1) % 8 == 0) {
			printf("\n");
		}
	}
	printf("\n*******************************\n");

}

