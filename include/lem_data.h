
#ifndef DCPU_EMULATOR_LEM_DATA_H
#define DCPU_EMULATOR_LEM_DATA_H

#include <stdint.h>

#define LEM1802_PALETTE_SIZE 16
#define LEM1802_FONT_SIZE    256
#define LEM1802_BLINK_RATE_MS 500

extern const uint16_t lem1802_default_palette[LEM1802_PALETTE_SIZE];
extern const uint16_t lem1802_default_font[LEM1802_FONT_SIZE];

#endif // DCPU_EMULATOR_LEM1802_DATA_H