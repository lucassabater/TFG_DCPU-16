#include <stdio.h>
#include <SDL2/SDL.h>

#include "emulation_utils.h"
#include "emulator.h"
#include "window.h"

int main(int argc, char* argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("Error SDL: %s\n", SDL_GetError());
        return -1;
    }

    EmulatorState emu;
    emu_init(&emu);

    MainWindow window;
    if (!window_init(&window)) {
        printf("Error creando la ventana.\n");
        return -1;
    }

    bool running = true;
    while (running) {

        running = window_process_events(&window, &emu);

        emu_step_frame(&emu);
        emu_update_key_repeat(&emu);

        window_render(&window, &emu);
    }

    // 4. Limpieza
    window_cleanup(&window);
    SDL_Quit();

    return 0;
}