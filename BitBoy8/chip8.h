#pragma once
#define NUM_REGISTERS 16
#define STACK_LEVELS 16
#define KEYPAD_SIZE 16
#define SIZE_MEMORY 4096
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define ROM_START 0x200
#define FONTSET_START_ADDRESS 0x50
#define FONTSET_SIZE 80
#define WIDTH_SPRITE 8


//xpos = i % 64, ypos = y / 64
extern uint8_t video[SCREEN_WIDTH * SCREEN_HEIGHT];
extern uint8_t memory[SIZE_MEMORY];

uint8_t get_random_byte();
uint8_t get_bit_from_byte(uint8_t byte, uint8_t bit);
uint8_t get_pixel_set_in_display(uint8_t xpos, uint8_t ypos);
uint16_t get_pixel_array_index_in_display(uint8_t xpos, uint8_t ypos);

void op_1NNN();

void op_00EO();

void op_00EE();

void op_2NNN();

void op_3XNN();

void op_4XNN();

void op_5XY0();

void op_6XNN();

void op_7XNN();

void op_8XY0();

void op_8XY1();

void op_8XY2();

void op_8XY3();

void op_8XY4();

void op_8XY5();

void op_8XY6();

void op_8XY7();

void op_8XYE();

void op_9XY0();

void op_ANNN();

void op_BNNN();

void op_DXYN();

void emulate_cycle();

void load_font();

void init_chip8();

