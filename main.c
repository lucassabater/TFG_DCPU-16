/**
 * @file main.c
 * @brief Entry point and main execution loop for the DCPU-16 emulator.
 */

#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "emulation_utils.h"
#include "emulator.h"
#include "window.h"
#define unlikely(x)     __builtin_expect(!!(x), 0)
#define MAX_DELTA_TIME 0.1

/**
 * @brief Application entry point.
 * Initializes subsystems and runs the cycle-accurate execution loop.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on success, non-zero on failure.
 */
int main(int argc, char* argv[]) {
    // Initialize core subsystems
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    EmulatorState emu;
    emu_init(&emu);

    MainWindow window;
    if (!window_init(&window)) {
        fprintf(stderr, "Window initialization failed.\n");
        SDL_Quit();
        return -1;
    }

    // Timing and cycle accumulation state
    Uint64 current_time = SDL_GetPerformanceCounter();
    Uint64 last_time = 0;
    double cycle_accumulator = 0.0;

    bool running = true;

    while (running) {
        running = window_process_events(&window, &emu);

        // Delta time calculation
        last_time = current_time;
        current_time = SDL_GetPerformanceCounter();
        double delta_time = (double)(current_time - last_time) / (double)SDL_GetPerformanceFrequency();

        // Prevent spiral of death on init
        if (delta_time > MAX_DELTA_TIME) {
            delta_time = MAX_DELTA_TIME;
        }

        if (emu.is_playing) {
            cycle_accumulator += delta_time * emu.speed_hz;

            while (cycle_accumulator >= 1.0) {

                if (unlikely(emu.cpu.is_on_fire)) {
                    emu.is_playing = false;
                    cycle_accumulator = 0.0;
                    break;
                }

                uint32_t cycles_taken = dcpu_step(&emu.cpu);

                // Failsafe: prevent infinite loops
                if (cycles_taken == 0) {
                    cycles_taken = 1;
                }

                cycle_accumulator -= cycles_taken;

                // Sync hardware peripherals to CPU cycles
                clock_tick(&emu.clock, &emu.cpu, cycles_taken);

                //cpu_dump(&emu.cpu);
            }
        } else {
            cycle_accumulator = 0.0;
        }

        emu_update_key_repeat(&emu);

        window_render(&window, &emu);
    }

    // Cleanup and exit
    window_cleanup(&window);
    SDL_Quit();

    return 0;
}