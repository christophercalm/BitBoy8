#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <SDL.h>
#include "chip8.h"


#define SDL_SCALE_FACTOR 15
#define SDL_SCREEN_WIDTH (SCREEN_WIDTH * SDL_SCALE_FACTOR)
#define SDL_SCREEN_HEIGHT (SCREEN_HEIGHT * SDL_SCALE_FACTOR)


int main(int argc, char* argv);
void load_rom(unsigned char* buffer, int32_t fsize);
void load_font();
void update_graphics(SDL_Window **window, SDL_Surface **screen_surface);
void init_graphics(SDL_Window  **window, SDL_Surface  **screen_surface);
int get_instruction(unsigned char* codeBuffer, int pc);


int main(int argc, char* argv) {
    //The window we'll be rendering to
    SDL_Window* window = NULL;

    //The surface contained by the window
    SDL_Surface* screen_surface = NULL;

    init_graphics(&window, &screen_surface);

    init_chip8();
    printf("%02x", get_random_byte());

    FILE* f = fopen("IBM.ch8", "rb");

    //FILE *f = fopen(argv[1], "rb");
    if (f == NULL) {
        printf("error: Couldn't open %c\n", argv[1]);
        exit(1);
    }

    //get the file size and read it into a memory buffer
    fseek(f, 0L, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);
    unsigned char* buffer = malloc(fsize);

    fread(buffer, fsize, 1, f);
    fclose(f);
    load_rom(buffer, fsize);

    while (1) {
        emulate_cycle();
        update_graphics(&window, &screen_surface);
    }

    SDL_DestroyWindow(window);

    SDL_Quit();    
    return 0;
}


void init_graphics(SDL_Window **window, SDL_Surface **screen_surface) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }

    *window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SDL_SCREEN_WIDTH, SDL_SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    }
    else {
        *screen_surface = SDL_GetWindowSurface(*window);
        //Fill the surface black
        SDL_FillRect(*screen_surface, NULL, SDL_MapRGB((*screen_surface)->format, 0x00, 0x00, 0x00));

        SDL_UpdateWindowSurface(*window);
    }
}

void update_graphics(SDL_Window **window, SDL_Surface **screen_surface) {
    //video[0] = 1;
    video[2047] = 1;
    for (uint16_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
        if (video[i] == 1) {
            uint16_t x_pos = (i % SCREEN_WIDTH) * SDL_SCALE_FACTOR;
            uint16_t yoff = i / SCREEN_WIDTH;
            uint16_t y_pos = (i / SCREEN_WIDTH) * SDL_SCALE_FACTOR;
            SDL_Rect rect = { x_pos, y_pos, SDL_SCALE_FACTOR, SDL_SCALE_FACTOR }; // x, y, width, height
            SDL_FillRect(*screen_surface, &rect, SDL_MapRGB((*screen_surface)->format, 0xFF, 0xFF, 0xFF));
        }
    }
    SDL_UpdateWindowSurface(*window);
    //SDL_Delay(4000);
}


void load_rom(unsigned char* buffer, int32_t fsize) {
    for (uint32_t i = 0; i < fsize; ++i) {
        memory[ROM_START + i] = buffer[i];
    }
    free(buffer);
}


/**
 * *codebuffer is a pointer to z80 code
 *  pc is offset in the code
 */
int get_instruction(unsigned char* codeBuffer, int pc) {
    unsigned char* code = &codeBuffer[pc]; //gets the code at the offset at pc
    int opBytes = 1;
    printf("%02x ", code[0]);
    return 1;
}





