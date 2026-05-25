//
// Created by lucas on 25/05/2026.
//

#ifndef DCPU_EMULATOR_WINDOW_H
#define DCPU_EMULATOR_WINDOW_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "emulator.h"

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *lem_texture;
    struct nk_context *ctx;

    int window_w;
    int window_h;
} MainWindow;

bool window_init(MainWindow *win);
void window_cleanup(MainWindow *win);

bool window_process_events(MainWindow *win, EmulatorState *emu);
void window_render(MainWindow *win, EmulatorState *emu);

#endif //DCPU_EMULATOR_WINDOW_H