/**
* @file lem_data.h
 * @brief Default ROM data definitions for the LEM1802 display.
 */

#ifndef LEM_DATA_H
#define LEM_DATA_H

#include <stdint.h>

#define LEM1802_PALETTE_SIZE 16
#define LEM1802_FONT_SIZE    256
#define LEM1802_BLINK_RATE_MS 500

extern const uint16_t lem1802_default_palette[LEM1802_PALETTE_SIZE];
extern const uint16_t lem1802_default_font[LEM1802_FONT_SIZE];

#endif // LEM_DATA_H