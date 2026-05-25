//
// Created by lucas on 25/05/2026.
//

#ifndef DCPU_EMULATOR_EMULATOR_H
#define DCPU_EMULATOR_EMULATOR_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL_keycode.h> // Solo para los Keycodes

#include "dcpu16.h"
#include "generic_clock.h"
#include "keyboard.h"
#include "lem1802.h"

typedef struct {
    DCPU16 cpu;
    GenericClock clock;
    GenericKeyboard keyboard;
    LEM1802 screen;

    bool is_playing;
    bool rom_loaded;
    int speed_hz;
    uint16_t held_key;
    uint32_t hold_start_time;
    uint32_t last_repeat_time;

} EmulatorState;

// Funciones principales
void emu_init(EmulatorState *emu);
void emu_reset(EmulatorState *emu);
void emu_load_rom(EmulatorState *emu, const char *filepath);
void emu_step_frame(EmulatorState *emu);

// Manejo de teclado
void emu_handle_keydown(EmulatorState *emu, SDL_Keycode key);
void emu_handle_keyup(EmulatorState *emu, SDL_Keycode key);
void emu_update_key_repeat(EmulatorState *emu);

#endif //DCPU_EMULATOR_EMULATOR_H