/**
* @file keyboard.h
 * @brief Hardware definition for the DCPU-16 Generic Keyboard.
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware_device.h"

#define KEYBOARD_HARDWARE_ID      0x30CF7406
#define KEYBOARD_HARDWARE_VERSION 0x0001
#define KEYBOARD_MANUFACTURER     0x1C6C8B36

#define KEYBOARD_BUFFER_SIZE      64
#define KEYBOARD_CHARACTERS       0x100

// DCPU-16 specific keycodes
#define KEY_BACKSPACE        0x10
#define KEY_RETURN           0x11
#define KEY_INSERT           0x12
#define KEY_DELETE           0x13
#define KEY_ARROW_UP         0x80
#define KEY_ARROW_DOWN       0x81
#define KEY_ARROW_LEFT       0x82
#define KEY_ARROW_RIGHT      0x83
#define KEY_SHIFT            0x90
#define KEY_CONTROL          0x91

typedef struct {
    DCPU_Hardware base;

    uint16_t buffer[KEYBOARD_BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;

    uint16_t int_message;
    bool is_pressed[KEYBOARD_CHARACTERS];
} GenericKeyboard;

void keyboard_init(GenericKeyboard *kb);
void keyboard_press_key(GenericKeyboard *kb, DCPU16 *cpu, uint16_t key_code);
void keyboard_release_key(GenericKeyboard *kb, DCPU16 *cpu, uint16_t key_code);
void keyboard_clear_buffer(GenericKeyboard *kb);

#endif // KEYBOARD_H