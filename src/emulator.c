//
// Created by lucas on 25/05/2026.
//

#include "emulator.h"
#include "emulation_utils.h"

#define KEY_REPEAT_DELAY_MS 400
#define KEY_REPEAT_RATE_MS 50

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

void emu_init(EmulatorState *emu) {
    emu->is_playing = false;
    emu->rom_loaded = false;
    emu->speed_hz = 100000;

    dcpu_init(&emu->cpu);
    clock_init(&emu->clock);
    keyboard_init(&emu->keyboard);
    lem1802_init(&emu->screen);

    dcpu_connect_hardware(&emu->cpu, (DCPU_Hardware*)&emu->screen);
    dcpu_connect_hardware(&emu->cpu, (DCPU_Hardware*)&emu->keyboard);
    dcpu_connect_hardware(&emu->cpu, (DCPU_Hardware*)&emu->clock);
}

void emu_reset(EmulatorState *emu) {
    dcpu_init(&emu->cpu);
    // Hay que reconectar el hardware tras un reset completo
    dcpu_connect_hardware(&emu->cpu, (DCPU_Hardware*)&emu->screen);
    dcpu_connect_hardware(&emu->cpu, (DCPU_Hardware*)&emu->keyboard);
    dcpu_connect_hardware(&emu->cpu, (DCPU_Hardware*)&emu->clock);

    emu->is_playing = false; // Paramos por seguridad
}

void emu_load_rom(EmulatorState *emu, const char *filepath) {
    emu_reset(emu);
    if (load_rom(&emu->cpu, filepath)) {
        emu->rom_loaded = true;
        emu->is_playing = true; // Auto-play
    }
}

void emu_step_frame(EmulatorState *emu) {
    if (emu->is_playing && emu->rom_loaded && !emu->cpu.halted) {
        uint32_t target_cycles = emu->speed_hz / 60;
        uint32_t cycles_this_frame = 0;

        while (cycles_this_frame < target_cycles && !emu->cpu.halted) {
            uint32_t c = dcpu_step(&emu->cpu);
            cycles_this_frame += c;
            clock_tick(&emu->clock, &emu->cpu, c);
        }
    }
}

void emu_handle_keydown(EmulatorState *emu, SDL_Keycode key) {
    uint16_t dcpu_key = translate_key(key);
    if (dcpu_key) {
        keyboard_press_key(&emu->keyboard, &emu->cpu, dcpu_key);

        // Empezamos a cronometrar esta tecla
        emu->held_key = dcpu_key;
        emu->hold_start_time = SDL_GetTicks();
        emu->last_repeat_time = SDL_GetTicks();
    }
}

void emu_handle_keyup(EmulatorState *emu, SDL_Keycode key) {
    uint16_t dcpu_key = translate_key(key);
    if (dcpu_key) {
        keyboard_release_key(&emu->keyboard, &emu->cpu, dcpu_key);

        // Si el usuario soltó la tecla que estábamos repitiendo, cancelamos el cronómetro
        if (emu->held_key == dcpu_key) {
            emu->held_key = 0;
        }
    }
}

// El motor de repetición personalizado
void emu_update_key_repeat(EmulatorState *emu) {
    if (emu->held_key != 0 && emu->is_playing) {
        uint32_t now = SDL_GetTicks();

        // 1. ¿Ya ha pasado la pausa inicial (Delay)?
        if (now - emu->hold_start_time > KEY_REPEAT_DELAY_MS) {
            // 2. ¿Toca meter otra letra según nuestro Rate?
            if (now - emu->last_repeat_time > KEY_REPEAT_RATE_MS) {
                keyboard_press_key(&emu->keyboard, &emu->cpu, emu->held_key);
                emu->last_repeat_time = now; // Reiniciamos el cronómetro de repetición
            }
        }
    }
}