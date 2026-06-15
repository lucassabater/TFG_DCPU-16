/**
* @file window.h
 * @brief Window management, event polling, and GUI definitions.
 */

#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include "emulator.h"

// Forward declaration for Nuklear context to avoid including nuklear.h
struct nk_context;

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

#endif // WINDOW_H