/**
 * @file emulator.c
 * @brief High-level emulator state management and hardware integration.
 */

#include "emulator.h"
#include "emulation_utils.h"

#define KEY_REPEAT_DELAY_MS 400
#define KEY_REPEAT_RATE_MS 50

/**
 * @brief Translates an SDL keycode to the corresponding DCPU-16 keyboard spec code.
 */
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

/**
 * @brief Initializes the emulator state and attaches default hardware.
 * * @param emu Pointer to the EmulatorState instance.
 */
void emu_init(EmulatorState *emu) {
    emu->rom_loaded = false;
    emu->speed_hz = 100000;

    emu_reset(emu);
}

/**
 * @brief Performs a reset on the virtual machine and all peripherals.
 * * @param emu Pointer to the EmulatorState instance.
 */
void emu_reset(EmulatorState *emu) {
    dcpu_init(&emu->cpu);

    clock_init(&emu->clock);
    keyboard_init(&emu->keyboard);
    lem1802_init(&emu->screen);

    dcpu_add_hardware(&emu->cpu, (DCPU_Hardware*)&emu->screen);
    dcpu_add_hardware(&emu->cpu, (DCPU_Hardware*)&emu->keyboard);
    dcpu_add_hardware(&emu->cpu, (DCPU_Hardware*)&emu->clock);

    emu->is_playing = false;
}


/**
 * @brief Loads a ROM and triggers auto-play on success.
 * * @param emu Pointer to the EmulatorState instance.
 * @param filepath Path to the ROM file.
 */
void emu_load_rom(EmulatorState *emu, const char *filepath) {
    emu_reset(emu);
    if (load_rom(&emu->cpu, filepath)) {
        emu->rom_loaded = true;
        emu->is_playing = true;
    }
}

/**
 * @brief Handles key press events and initializes the key repeat timer.
 * * @param emu Pointer to the EmulatorState instance.
 * @param key SDL keycode.
 */
void emu_handle_keydown(EmulatorState *emu, SDL_Keycode key) {
    uint16_t dcpu_key = translate_key(key);
    if (dcpu_key) {
        keyboard_press_key(&emu->keyboard, &emu->cpu, dcpu_key);

        emu->held_key = dcpu_key;
        emu->hold_start_time = SDL_GetTicks();
        emu->last_repeat_time = SDL_GetTicks();
    }
}

/**
 * @brief Handles key release events and cancels the repeat timer if applicable.
 * * @param emu Pointer to the EmulatorState instance.
 * @param key SDL keycode.
 */
void emu_handle_keyup(EmulatorState *emu, SDL_Keycode key) {
    uint16_t dcpu_key = translate_key(key);
    if (dcpu_key) {
        keyboard_release_key(&emu->keyboard, &emu->cpu, dcpu_key);

        if (emu->held_key == dcpu_key) {
            emu->held_key = 0;
        }
    }
}

/**
 * @brief Processes the custom key repeat logic based on host elapsed time.
 * * @param emu Pointer to the EmulatorState instance.
 */
void emu_update_key_repeat(EmulatorState *emu) {
    if (emu->held_key != 0 && emu->is_playing) {
        uint32_t now = SDL_GetTicks();

        if (now - emu->hold_start_time > KEY_REPEAT_DELAY_MS) {
            if (now - emu->last_repeat_time > KEY_REPEAT_RATE_MS) {
                keyboard_press_key(&emu->keyboard, &emu->cpu, emu->held_key);
                emu->last_repeat_time = now;
            }
        }
    }
}