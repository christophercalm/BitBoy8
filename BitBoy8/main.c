#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <SDL.h>
#include "chip8.h"



#define SDL_SCALE_FACTOR 20
#define SDL_SCREEN_WIDTH (SCREEN_WIDTH * SDL_SCALE_FACTOR)
#define SDL_SCREEN_HEIGHT (SCREEN_HEIGHT * SDL_SCALE_FACTOR)
#define DEBUG 1
#define DELAY_TIME 4


int main(int argc, char* argv);
void load_rom(unsigned char* buffer, int32_t fsize);
void load_font();
void update_graphics(SDL_Window **window, SDL_Surface **screen_surface);
void init_graphics(SDL_Window  **window, SDL_Surface  **screen_surface);

int main(int argc, char* argv) {
    uint8_t quit = 0;
    //The window we'll be rendering to
    SDL_Window* window = NULL;

    //The surface contained by the window
    SDL_Surface* screen_surface = NULL;

    //timing variables to slow down execution
    uint32_t current_time = 0;
    uint32_t last_time = 0;

    SDL_Event key_event;

    init_graphics(&window, &screen_surface);

    init_chip8();

    FILE* f = fopen("breakout.rom", "rb");

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

    while (!quit) {
        while (SDL_PollEvent(&key_event) != 0) {
            /* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
            if (key_event.type == SDL_QUIT) {
                quit = 1;
            }
            else if (key_event.type == SDL_KEYDOWN) {
                switch (key_event.key.keysym.sym) {
                case SDLK_1: keypad[0x1] = 1; break;
                case SDLK_2: keypad[0x2] = 1; break;
                case SDLK_3: keypad[0x3] = 1; break;
                case SDLK_4: keypad[0xC] = 1; break;
                case SDLK_q: keypad[0x4] = 1; break;
                case SDLK_w: keypad[0x5] = 1; break;
                case SDLK_e: keypad[0x6] = 1; break;
                case SDLK_r: keypad[0xD] = 1; break;
                case SDLK_a: keypad[0x7] = 1; break;
                case SDLK_s: keypad[0x8] = 1; break;
                case SDLK_d: keypad[0x9] = 1; break;
                case SDLK_f: keypad[0xE] = 1; break;
                case SDLK_z: keypad[0xA] = 1; break;
                case SDLK_x: keypad[0x0] = 1; break;
                case SDLK_c: keypad[0xB] = 1; break;
                case SDLK_v: keypad[0xF] = 1; break;
                default:
                    break;
                }
            }
            else if (key_event.type == SDL_KEYUP) {
                switch (key_event.key.keysym.sym) {
                case SDLK_1: keypad[0x1] = 0; break;
                case SDLK_2: keypad[0x2] = 0; break;
                case SDLK_3: keypad[0x3] = 0; break;
                case SDLK_4: keypad[0xC] = 0; break;
                case SDLK_q: keypad[0x4] = 0; break;
                case SDLK_w: keypad[0x5] = 0; break;
                case SDLK_e: keypad[0x6] = 0; break;
                case SDLK_r: keypad[0xD] = 0; break;
                case SDLK_a: keypad[0x7] = 0; break;
                case SDLK_s: keypad[0x8] = 0; break;
                case SDLK_d: keypad[0x9] = 0; break;
                case SDLK_f: keypad[0xE] = 0; break;
                case SDLK_z: keypad[0xA] = 0; break;
                case SDLK_x: keypad[0x0] = 0; break;
                case SDLK_c: keypad[0xB] = 0; break;
                case SDLK_v: keypad[0xF] = 0; break;
                default:
                    break;
                }
            }
            
        }
        current_time = SDL_GetTicks();
        if (current_time > last_time + DELAY_TIME) {
            emulate_cycle();
            update_graphics(&window, &screen_surface);
            last_time = current_time;
        }
 
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

    *window = SDL_CreateWindow("BitBoy8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SDL_SCREEN_WIDTH, SDL_SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
    if (draw_flag) {
        for (uint16_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
            uint16_t x_pos = (i % SCREEN_WIDTH) * SDL_SCALE_FACTOR;
            uint16_t yoff = i / SCREEN_WIDTH;
            uint16_t y_pos = (i / SCREEN_WIDTH) * SDL_SCALE_FACTOR;
            SDL_Rect rect = { x_pos, y_pos, SDL_SCALE_FACTOR, SDL_SCALE_FACTOR }; // x, y, width, height
            if (video[i] == 1) {
                SDL_FillRect(*screen_surface, &rect, SDL_MapRGB((*screen_surface)->format, 0xFF, 0xFF, 0xFF));
            }
            else {
                SDL_FillRect(*screen_surface, &rect, SDL_MapRGB((*screen_surface)->format, 0x00, 0x00, 0x00));
            }
        }
        SDL_UpdateWindowSurface(*window);
    }
    draw_flag = 0;
}


void load_rom(unsigned char* buffer, int32_t fsize) {
    for (uint32_t i = 0; i < fsize; ++i) {
        memory[ROM_START + i] = buffer[i];
    }
    free(buffer);
}





