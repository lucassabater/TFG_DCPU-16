//
// Created by lucas on 13/05/2026.
//

#include "../include/keyboard.h"

static void handle_hwi(DCPU_Hardware *hw, DCPU16 *cpu) {
    GenericKeyboard *kb = (GenericKeyboard *)hw;

    switch (cpu->reg[0]) {
        case 0: // CLEAR_BUFFER
            kb->head = 0;
            kb->tail = 0;
            kb->count = 0;
            break;

        case 1: // GET_NEXT_KEY
            if (kb->count > 0) {
                cpu->reg[2] = kb->buffer[kb->tail]; // Registro C = tecla
                kb->tail = (kb->tail + 1) % KEYBOARD_BUFFER_SIZE;
                kb->count--;
            } else {
                cpu->reg[2] = 0; // Si no hay teclas, devuelve 0
            }
            break;

        case 2: // CHECK_KEY_PRESSED
            // El registro B contiene el código de tecla a comprobar
            if (cpu->reg[1] < 0x100) {
                cpu->reg[2] = kb->is_pressed[cpu->reg[1]] ? 1 : 0; // Registro C = 1 si está pulsada
            } else {
                cpu->reg[2] = 0;
            }
            break;

        case 3: // SET_INTERRUPT_MESSAGE
            kb->int_message = cpu->reg[1]; // Registro B
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
    if (key_code >= 0x100) return; // Evitar desbordar el array de estados

    // 1. Guardar en el buffer circular si hay espacio
    if (kb->count < KEYBOARD_BUFFER_SIZE) {
        kb->buffer[kb->head] = key_code;
        kb->head = (kb->head + 1) % KEYBOARD_BUFFER_SIZE;
        kb->count++;
    }

    // 2. Registrar que la tecla está actualmente presionada
    kb->is_pressed[key_code] = true;

    // 3. Si hay un mensaje de interrupción configurado, avisamos a la CPU
    if (kb->int_message != 0) {
        interrupt_enqueue(cpu, kb->int_message);
    }
}

void keyboard_release_key(GenericKeyboard *kb, DCPU16 *cpu, uint16_t key_code) {
    if (key_code >= 0x100) return;

    // Registrar que la tecla se ha soltado
    kb->is_pressed[key_code] = false;

    // Según la Spec, soltar una tecla TAMBIÉN lanza la interrupción
    // si esta está habilitada (el software leerá el estado actual mediante HWI 2)
    if (kb->int_message != 0) {
        interrupt_enqueue(cpu, kb->int_message);
    }
}