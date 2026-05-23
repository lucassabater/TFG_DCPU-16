#ifndef DCPU_EMULATOR_LEM1802_H
#define DCPU_EMULATOR_LEM1802_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "hardware_device.h"
#include "dcpu16.h"

#define LEM1802_HARDWARE_ID      0x7349f615
#define LEM1802_HARDWARE_VERSION 0x1802
#define LEM1802_MANUFACTURER     0x1c6c8b36

#define LEM1802_COLS 32
#define LEM1802_ROWS 12
#define LEM1802_WIDTH  (LEM1802_COLS * 4) // 128
#define LEM1802_HEIGHT (LEM1802_ROWS * 8) // 96

// El struct mágico de Hellige adaptado a nuestra estructura
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

    // Sistema de renderizado optimizado
    TileCache cache[LEM1802_COLS * LEM1802_ROWS];
    uint32_t pixels[LEM1802_WIDTH * LEM1802_HEIGHT]; // Buffer local
    bool dirty;

    // Control de parpadeo exacto
    uint32_t last_blink_time;
    bool blink_state;
} LEM1802;

void lem1802_init(LEM1802 *monitor);
bool lem1802_update(LEM1802 *monitor, DCPU16 *cpu, SDL_Texture *texture);
uint32_t lem1802_get_border_color(LEM1802 *monitor, DCPU16 *cpu);

#endif