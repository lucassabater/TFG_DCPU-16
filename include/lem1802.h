/**
* @file lem1802.h
 * @brief Hardware definition for the LEM1802 Display peripheral.
 */

#ifndef LEM1802_H
#define LEM1802_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "hardware_device.h"

#define LEM1802_HARDWARE_ID      0x7349f615
#define LEM1802_HARDWARE_VERSION 0x1802
#define LEM1802_MANUFACTURER     0x1C6C8B36

#define LEM1802_COLS 32
#define LEM1802_ROWS 12
#define LEM1802_COL_FACTOR 4
#define LEM1802_ROW_FACTOR 8
#define LEM1802_WIDTH  (LEM1802_COLS * LEM1802_COL_FACTOR) // 128
#define LEM1802_HEIGHT (LEM1802_ROWS * LEM1802_ROW_FACTOR) // 96

typedef struct {
    uint32_t glyph;
    uint32_t fg;
    uint32_t bg;
} TileCache;

typedef struct {
    DCPU_Hardware base;

    uint16_t vram_map;
    uint16_t font_map;
    uint16_t palette_map;
    uint8_t border_color_idx;

    TileCache cache[LEM1802_COLS * LEM1802_ROWS];
    uint32_t pixels[LEM1802_WIDTH * LEM1802_HEIGHT];
    bool dirty;

    uint32_t last_blink_time;
    bool blink_state;
} LEM1802;

void lem1802_init(LEM1802 *monitor);
bool lem1802_update(LEM1802 *monitor, DCPU16 *cpu, SDL_Texture *texture);
uint32_t lem1802_get_border_color(LEM1802 *monitor, DCPU16 *cpu);

#endif // LEM1802_H