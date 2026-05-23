#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

// Tus módulos
#include "dcpu16.h"
#include "generic_clock.h"
#include "keyboard.h"
#include "lem1802.h"
#include "emulation_utils.h" // <-- Aquí está tu cargador de .hex

#define SCALE_FACTOR 6
#define BORDER_SIZE  8

static uint16_t translate_key(SDL_Keycode sdl_key) {
    if (sdl_key >= 0x20 && sdl_key <= 0x7f) return (uint16_t)sdl_key;
    switch (sdl_key) {
        case SDLK_BACKSPACE: return KEY_BACKSPACE;
        case SDLK_RETURN:    return KEY_RETURN;
        case SDLK_INSERT:    return KEY_INSERT;
        case SDLK_DELETE:    return KEY_DELETE;
        case SDLK_UP:        return KEY_ARROW_UP;
        case SDLK_DOWN:      return KEY_ARROW_DOWN;
        case SDLK_LEFT:      return KEY_ARROW_LEFT;
        case SDLK_RIGHT:     return KEY_ARROW_RIGHT;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:    return KEY_SHIFT;
        case SDLK_LCTRL:
        case SDLK_RCTRL:     return KEY_CONTROL;
        default:             return 0;
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("Error initializing SDL: %s\n", SDL_GetError());
        return -1;
    }

    int window_w = (LEM1802_WIDTH + BORDER_SIZE * 2) * SCALE_FACTOR;
    int window_h = (LEM1802_HEIGHT + BORDER_SIZE * 2) * SCALE_FACTOR;

    SDL_Window *window = SDL_CreateWindow("DCPU-16 Emulator",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          window_w, window_h,
                                          SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             LEM1802_WIDTH, LEM1802_HEIGHT);


    DCPU16 cpu;
    dcpu_init(&cpu);

    GenericClock mi_reloj;
    clock_init(&mi_reloj);

    GenericKeyboard mi_teclado;
    keyboard_init(&mi_teclado);

    LEM1802 mi_pantalla;
    lem1802_init(&mi_pantalla);


    connect_hardware(&cpu, (DCPU_Hardware*)&mi_pantalla);
    connect_hardware(&cpu, (DCPU_Hardware*)&mi_teclado);
    connect_hardware(&cpu, (DCPU_Hardware*)&mi_reloj);


    if (!load_rom(&cpu, "../test/tetris.bin")) {
        if (!load_rom(&cpu, "test/tetris.bin")) {
            printf("CRITICAL ERROR: Could not open pacman.hex\n");
            free_hardware(&cpu);
            SDL_Quit();
            return -1;
        }
    }

    bool running = true;
    SDL_Event event;
    const uint32_t CYCLES_PER_FRAME = 100000 / 60;

    while (running) {
        Uint32 frame_start_time = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                uint16_t dcpu_key = translate_key(event.key.keysym.sym);
                if (dcpu_key != 0) keyboard_press_key(&mi_teclado, &cpu, dcpu_key);
            }
            else if (event.type == SDL_KEYUP) {
                uint16_t dcpu_key = translate_key(event.key.keysym.sym);
                if (dcpu_key != 0) keyboard_release_key(&mi_teclado, &cpu, dcpu_key);
            }
        }

        uint32_t cycles_this_frame = 0;
        while (cycles_this_frame < CYCLES_PER_FRAME && !cpu.halted) {
            uint32_t c = dcpu_step(&cpu);
            cycles_this_frame += c;

            clock_tick(&mi_reloj, &cpu, c);
        }

        if (cpu.halted) {
            cycles_this_frame = CYCLES_PER_FRAME;
        }

        if (lem1802_update(&mi_pantalla, &cpu, texture)) {

            const uint32_t border_rgba = lem1802_get_border_color(&mi_pantalla, &cpu);
            const uint8_t r = (border_rgba >> 24) & 0xFF;
            const uint8_t g = (border_rgba >> 16) & 0xFF;
            const uint8_t b = (border_rgba >> 8)  & 0xFF;

            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderClear(renderer);

            SDL_Rect dst_rect;
            dst_rect.x = BORDER_SIZE * SCALE_FACTOR;
            dst_rect.y = BORDER_SIZE * SCALE_FACTOR;
            dst_rect.w = LEM1802_WIDTH * SCALE_FACTOR;
            dst_rect.h = LEM1802_HEIGHT * SCALE_FACTOR;

            SDL_RenderCopy(renderer, texture, NULL, &dst_rect);
        }
        SDL_RenderPresent(renderer);

        Uint32 frame_duration = SDL_GetTicks() - frame_start_time;
        if (frame_duration < 16) {
            SDL_Delay(16 - frame_duration);
        }
        //cpu_dump(&cpu);
    }

    free_hardware(&cpu);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}