//
// Created by lucas on 13/05/2026.
//

#include "keyboard.h"

#include <stdio.h>

static void handle_hwi(DCPU_Hardware *hw, DCPU16 *cpu) {
    GenericKeyboard *kb = (GenericKeyboard *)hw;

    switch (cpu->reg[A]) {
        case 0: // CLEAR_BUFFER
            kb->head = 0;
            kb->tail = 0;
            kb->count = 0;
            break;

        case 1: // GET_NEXT_KEY
            if (kb->count > 0) {
                cpu->reg[C] = kb->buffer[kb->tail];
                kb->tail = (kb->tail + 1) % KEYBOARD_BUFFER_SIZE;
                kb->count--;
            } else {
                cpu->reg[C] = 0;
            }
            break;

        case 2: // CHECK_KEY_PRESSED
            if (cpu->reg[B] < 0x100) {
                cpu->reg[C] = kb->is_pressed[cpu->reg[B]] ? 1 : 0;
            } else {
                cpu->reg[C] = 0;
            }
            break;

        case 3: // SET_INTERRUPT_MESSAGE
            kb->int_message = cpu->reg[B];
            break;

        default: break;
    }
}

void keyboard_init(GenericKeyboard *kb) {
    kb->base.hardware_id = KEYBOARD_HARDWARE_ID;
    kb->base.hardware_version = KEYBOARD_HARDWARE_VERSION;
    kb->base.manufacturer = KEYBOARD_MANUFACTURER;
    kb->base.handle_hwi = handle_hwi;

    kb->head = 0;
    kb->tail = 0;
    kb->count = 0;
    kb->int_message = 0;

    for (int i = 0; i < 0x100; i++) {
        kb->is_pressed[i] = false;
    }
}

void keyboard_press_key(GenericKeyboard *kb, DCPU16 *cpu, uint16_t key_code) {
    if (key_code >= 0x100) return;
    if (kb->count < KEYBOARD_BUFFER_SIZE) {
        kb->buffer[kb->head] = key_code;
        kb->head = (kb->head + 1) % KEYBOARD_BUFFER_SIZE;
        kb->count++;
    }

    kb->is_pressed[key_code] = true;

    if (kb->int_message != 0) {
        interrupt_enqueue(cpu, kb->int_message);
    }
}

void keyboard_release_key(GenericKeyboard *kb, DCPU16 *cpu, uint16_t key_code) {
    if (key_code >= 0x100) return;

    kb->is_pressed[key_code] = false;

    if (kb->int_message != 0) {
        interrupt_enqueue(cpu, kb->int_message);
    }
}