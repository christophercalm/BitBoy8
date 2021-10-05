#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "chip8.h"

uint8_t registers[NUM_REGISTERS] = { 0 };
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

uint8_t op_x;
uint8_t op_y;
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


void op_00EO() {
	memset(video, 0, sizeof(video));
}

void op_1NNN() {
	pc = nnn;
}

void op_00EE() {
	sp = stack[0];
	--sp;
}

void op_6XNN() {
	registers[op_x] = nn;
}

void op_7XNN() {
	registers[op_x] += nn;
}

void op_ANNN() {
	idx = nnn;
}

/**
 *  It will draw an N pixels tall sprite from the memory location that the I index register is holding to the screen,
 *  at the horizontal X coordinate in VX and the Y coordinate in VY. All the pixels that are “on” in the sprite will flip the pixels on the screen that it is drawn to.
 *  If any pixels on the screen were turned “off” by this, the VF flag register is set to 1. Otherwise, it’s set to 0.
 **/
void op_DXYN() {
	uint8_t x_pos = registers[op_x] % SCREEN_WIDTH;
	uint8_t y_pos = registers[op_y] % SCREEN_HEIGHT;
	registers[0xF] = 0;
	for (uint8_t row = 0; row < n; row++) {
		uint8_t display_byte = memory[idx + row];
		for (uint8_t sprite_pixel = 0; sprite_pixel < WIDTH_SPRITE; sprite_pixel++) {
			if (get_bit_from_byte(display_byte, WIDTH_SPRITE - sprite_pixel - 1) && get_pixel_set_in_display(x_pos, y_pos)) {
				video[get_pixel_array_index_in_display(x_pos, y_pos)] = 0;
				registers[0xF] = 1;
			}	else if (get_bit_from_byte(display_byte, WIDTH_SPRITE - sprite_pixel - 1) && !get_pixel_set_in_display(x_pos, y_pos)) {
				video[get_pixel_array_index_in_display(x_pos, y_pos)] = 1;
			}
			x_pos++;
		}
		x_pos = registers[op_x] % SCREEN_WIDTH;
		y_pos++;
		if (y_pos >= SCREEN_HEIGHT) {
			return;
		}
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
	op_x = memory[pc] & 0x0F;
	op_y = memory[pc + 1] >> 4;
	n = memory[pc + 1] & 0x0F;
	nn = memory[pc + 1];
	nnn = opcode & 0x0FFF;

	printf("%04x", opcode);

	//execute
	pc += 2;
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
			break;
		}
		break;
	case 0x1:
		op_1NNN();
		break;
	case 0x2:
		break;
	case 0x3:
		break;
	case 0x4:
		break;
	case 0x5:
		break;
	case 0x6:
		op_6XNN();
		break;
	case 0x7:
		op_7XNN();
		break;
	case 0x9:
		break;
	case 0xA:
		op_ANNN();
		break;
	case 0xB:
		break;
	case 0xC:
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
	uint8_t display_index = get_pixel_array_index_in_display(xpos, ypos);
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


