#include "keyboard.h"

#include <string.h>

#include "dcpu16.h"

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
            if (cpu->reg[B] < KEYBOARD_CHARACTERS) {
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

    memset(kb->is_pressed, 0, sizeof(kb->is_pressed));
}

void keyboard_press_key(GenericKeyboard *kb, DCPU16 *cpu, uint16_t key_code) {
    if (key_code >= KEYBOARD_CHARACTERS) return;
    if (kb->count < KEYBOARD_BUFFER_SIZE) {
        kb->buffer[kb->head] = key_code;
        kb->head = (kb->head + 1) % KEYBOARD_BUFFER_SIZE;
        kb->count++;
    }

    kb->is_pressed[key_code] = true;

    if (kb->int_message != 0) {
        dcpu_interrupt_enqueue(cpu, kb->int_message);
    }
}

void keyboard_release_key(GenericKeyboard *kb, DCPU16 *cpu, uint16_t key_code) {
    if (key_code >= KEYBOARD_CHARACTERS) return;

    kb->is_pressed[key_code] = false;

    if (kb->int_message != 0) {
        dcpu_interrupt_enqueue(cpu, kb->int_message);
    }
}