#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "chip8.h"

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
uint16_t opcode = 0;

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
}

//RET
void op_00EE() {
	--sp;
	pc = stack[pc];
}

//JP addr
void op_1NNN() {
	pc = nnn;
}

// call addr
void op_2NNN() {
	++sp;
	stack[0] = pc;
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

	V[x] = V[x] ^ V[y];
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
	if (get_bit_from_byte(V[x], 1)) {
		V[0xF] = 1;
	}
	else {
		V[0xF] = 0;
	}

	V[x] >>= 1;


}

//SUBN Vx, Vy
void op_8XY7() {
	if (V[x] > V[y]) {
		V[0xF] = 1;
	}
	else {
		V[0xF] = 0;
	}

	V[x] = V[y] - V[x];
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
	sp = nnn + V[0];
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
	uint8_t x_pos = V[x] % SCREEN_WIDTH;
	uint8_t y_pos = V[y] % SCREEN_HEIGHT;
	V[0xF] = 0;
	for (uint8_t row = 0; row < n; row++) {
		uint8_t display_byte = memory[idx + row];
		for (uint8_t sprite_pixel = 0; sprite_pixel < WIDTH_SPRITE; sprite_pixel++) {
			if (get_bit_from_byte(display_byte, WIDTH_SPRITE - sprite_pixel - 1) && get_pixel_set_in_display(x_pos, y_pos)) {
				video[get_pixel_array_index_in_display(x_pos, y_pos)] = 0;
				V[0xF] = 1;
			}	else if (get_bit_from_byte(display_byte, WIDTH_SPRITE - sprite_pixel - 1) && !get_pixel_set_in_display(x_pos, y_pos)) {
				video[get_pixel_array_index_in_display(x_pos, y_pos)] = 1;
			}
			x_pos++;
		}
		x_pos = V[x] % SCREEN_WIDTH; //reset x position
		y_pos++;
		if (y_pos >= SCREEN_HEIGHT) {
			return;
		}
	}
	//sleep(1);
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

	//execute
	pc += 2;
	//Mask off (with a binary AND) the first number in the instruction, and have one case per numbe

	uint16_t masked_opcode = (opcode & 0xF000) >> 12;
	uint8_t last_nibble_opcode = opcode & 0xF; 

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
	default:
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
	return SCREEN_WIDTH * ypos + xpos;
}

void init_chip8() {
	pc = ROM_START;
	load_font();
	srand(time(NULL));
}


