//
// Created by lucas on 13/05/2026.
//

#ifndef DCPU_EMULATOR_KEYBOARD_H
#define DCPU_EMULATOR_KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware_device.h"
#include "dcpu16.h"

#define KEYBOARD_HARDWARE_ID      0x30cf7406
#define KEYBOARD_HARDWARE_VERSION 0x0001
#define KEYBOARD_MANUFACTURER     0x00000000

#define KEYBOARD_BUFFER_SIZE      64

// Códigos de tecla especiales según la especificación DCPU-16 v1.7
#define DCPU_KEY_BACKSPACE        0x10
#define DCPU_KEY_RETURN           0x11
#define DCPU_KEY_INSERT           0x12
#define DCPU_KEY_DELETE           0x13
#define DCPU_KEY_ARROW_UP         0x80
#define DCPU_KEY_ARROW_DOWN       0x81
#define DCPU_KEY_ARROW_LEFT       0x82
#define DCPU_KEY_ARROW_RIGHT      0x83
#define DCPU_KEY_SHIFT            0x90
#define DCPU_KEY_CONTROL          0x91

typedef struct {
    DCPU_Hardware base;

    uint16_t buffer[KEYBOARD_BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;

    uint16_t int_message;
    bool is_pressed[0x100]; // Mantiene el estado de qué teclas están pulsadas (0x00-0xff)
} GenericKeyboard;

void keyboard_init(GenericKeyboard *kb);

// Funciones que llamará tu main.c cuando detecte eventos del sistema operativo
void keyboard_press_key(GenericKeyboard *kb, DCPU16 *cpu, uint16_t key_code);
void keyboard_release_key(GenericKeyboard *kb, DCPU16 *cpu, uint16_t key_code);

#endif