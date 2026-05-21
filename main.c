#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "dcpu16.h"
#include "generic_clock.h"
#include "keyboard.h"
// #include "lem1802.h" // El futuro monitor

#define LEM_WIDTH 128
#define LEM_HEIGHT 96
#define SCALE_FACTOR 4

// Función traductora de SDL a DCPU (como vimos antes)
uint16_t translate_key(SDL_Keycode sdl_key) {
    if (sdl_key >= 0x20 && sdl_key <= 0x7f) return (uint16_t)sdl_key;
    switch (sdl_key) {
        case SDLK_BACKSPACE: return DCPU_KEY_BACKSPACE;
        case SDLK_RETURN:    return DCPU_KEY_RETURN;
        case SDLK_INSERT:    return DCPU_KEY_INSERT;
        case SDLK_DELETE:    return DCPU_KEY_DELETE;
        case SDLK_UP:        return DCPU_KEY_ARROW_UP;
        case SDLK_DOWN:      return DCPU_KEY_ARROW_DOWN;
        case SDLK_LEFT:      return DCPU_KEY_ARROW_LEFT;
        case SDLK_RIGHT:     return DCPU_KEY_ARROW_RIGHT;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:    return DCPU_KEY_SHIFT;
        case SDLK_LCTRL:
        case SDLK_RCTRL:     return DCPU_KEY_CONTROL;
        default:             return 0;
    }
}

int main(int argc, char* argv[]) {
    // 1. INICIALIZAR SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("Error iniciando SDL: %s\n", SDL_GetError());
        return -1;
    }

    // 2. CREAR LA VENTANA
    SDL_Window *window = SDL_CreateWindow("Emulador DCPU-16 (TFG Lucas)",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          LEM_WIDTH * SCALE_FACTOR,
                                          LEM_HEIGHT * SCALE_FACTOR,
                                          SDL_WINDOW_SHOWN);

    // 3. CREAR EL RENDERER
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // 4. CREAR LA TEXTURA
    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             LEM_WIDTH, LEM_HEIGHT);

    // --- INICIALIZAR TU HARDWARE AQUÍ ---
    DCPU16 cpu;
    dcpu16_init(&cpu); // O dcpu_init, dependiendo de cómo la llamaras en dcpu16.c

    GenericClock mi_reloj;
    clock_init(&mi_reloj);

    GenericKeyboard mi_teclado;
    keyboard_init(&mi_teclado);

    // (Opcional) Si tienes una función para "conectar" hardware al bus de la CPU:
    // dcpu16_add_hardware(&cpu, (DCPU_Hardware*)&mi_reloj);
    // dcpu16_add_hardware(&cpu, (DCPU_Hardware*)&mi_teclado);

    // 5. EL BUCLE PRINCIPAL (Game Loop)
    bool running = true;
    SDL_Event event;

    // Frecuencia objetivo: 100 kHz. FPS objetivo: 60
    const uint32_t CYCLES_PER_FRAME = 100000 / 60; // Aprox 1666 ciclos

    while (running) {
        // Guardamos el tiempo al inicio del frame
        Uint32 frame_start_time = SDL_GetTicks();

        // A. GESTIÓN DE EVENTOS
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

        // B. EJECUCIÓN DE LA CPU Y RELOJ
        uint32_t cycles_this_frame = 0;
        while (cycles_this_frame < CYCLES_PER_FRAME) {
            uint32_t c = dcpu_step(&cpu); // Ejecuta 1 instrucción y devuelve lo que tardó
            cycles_this_frame += c;

            clock_tick(&mi_reloj, &cpu, c);
        }

        // C. RENDERIZADO
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        // D. CONTROL DE TIEMPO (Throttling a 60 FPS)
        // SDL_GetTicks() devuelve milisegundos. Un frame a 60 FPS debe durar ~16.6 ms.
        Uint32 frame_duration = SDL_GetTicks() - frame_start_time;
        if (frame_duration < 16) {
            SDL_Delay(16 - frame_duration); // Dormimos la CPU real lo que sobre
        }
    }

    // 6. LIMPIEZA FINAL
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}