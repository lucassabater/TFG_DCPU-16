#include "lem1802.h"
#include <string.h>

// Las paletas de Notch
static const uint16_t default_palette[16] = {
    0x0000, 0x000a, 0x00a0, 0x00aa, 0x0a00, 0x0a0a, 0x0a50, 0x0aaa,
    0x0555, 0x055f, 0x05f5, 0x05ff, 0x0f55, 0x0f5f, 0x0ff5, 0x0fff
};
static const uint16_t default_font[256] = {
    0xb79e, 0x388e, 0x722c, 0x75f4, 0x19bb, 0x7f8f, 0x85f9, 0xb158,
    0x242e, 0x2400, 0x082a, 0x0800, 0x0008, 0x0000, 0x0808, 0x0808,
    0x00ff, 0x0000, 0x00f8, 0x0808, 0x08f8, 0x0000, 0x080f, 0x0000,
    0x000f, 0x0808, 0x00ff, 0x0808, 0x08f8, 0x0808, 0x08ff, 0x0000,
    0x080f, 0x0808, 0x08ff, 0x0808, 0x6633, 0x99cc, 0x9933, 0x66cc,
    0xfef8, 0xe080, 0x7f1f, 0x0701, 0x0107, 0x1f7f, 0x80e0, 0xf8fe,
    0x5500, 0xaa00, 0x55aa, 0x55aa, 0xffaa, 0xff55, 0x0f0f, 0x0f0f,
    0xf0f0, 0xf0f0, 0x0000, 0xffff, 0xffff, 0x0000, 0xffff, 0xffff,
    0x0000, 0x0000, 0x005f, 0x0000, 0x0300, 0x0300, 0x3e14, 0x3e00,
    0x266b, 0x3200, 0x611c, 0x4300, 0x3629, 0x7650, 0x0002, 0x0100,
    0x1c22, 0x4100, 0x4122, 0x1c00, 0x1408, 0x1400, 0x081c, 0x0800,
    0x4020, 0x0000, 0x0808, 0x0800, 0x0040, 0x0000, 0x601c, 0x0300,
    0x3e49, 0x3e00, 0x427f, 0x4000, 0x6259, 0x4600, 0x2249, 0x3600,
    0x0f08, 0x7f00, 0x2745, 0x3900, 0x3e49, 0x3200, 0x6119, 0x0700,
    0x3649, 0x3600, 0x2649, 0x3e00, 0x0024, 0x0000, 0x4024, 0x0000,
    0x0814, 0x2241, 0x1414, 0x1400, 0x4122, 0x1408, 0x0259, 0x0600,
    0x3e59, 0x5e00, 0x7e09, 0x7e00, 0x7f49, 0x3600, 0x3e41, 0x2200,
    0x7f41, 0x3e00, 0x7f49, 0x4100, 0x7f09, 0x0100, 0x3e41, 0x7a00,
    0x7f08, 0x7f00, 0x417f, 0x4100, 0x2040, 0x3f00, 0x7f08, 0x7700,
    0x7f40, 0x4000, 0x7f06, 0x7f00, 0x7f01, 0x7e00, 0x3e41, 0x3e00,
    0x7f09, 0x0600, 0x3e41, 0xbe00, 0x7f09, 0x7600, 0x2649, 0x3200,
    0x017f, 0x0100, 0x3f40, 0x3f00, 0x1f60, 0x1f00, 0x7f30, 0x7f00,
    0x7708, 0x7700, 0x0778, 0x0700, 0x7149, 0x4700, 0x007f, 0x4100,
    0x031c, 0x6000, 0x0041, 0x7f00, 0x0201, 0x0200, 0x8080, 0x8000,
    0x0001, 0x0200, 0x2454, 0x7800, 0x7f44, 0x3800, 0x3844, 0x2800,
    0x3844, 0x7f00, 0x3854, 0x5800, 0x087e, 0x0900, 0x4854, 0x3c00,
    0x7f04, 0x7800, 0x447d, 0x4000, 0x2040, 0x3d00, 0x7f10, 0x6c00,
    0x417f, 0x4000, 0x7c18, 0x7c00, 0x7c04, 0x7800, 0x3844, 0x3800,
    0x7c14, 0x0800, 0x0814, 0x7c00, 0x7c04, 0x0800, 0x4854, 0x2400,
    0x043e, 0x4400, 0x3c40, 0x7c00, 0x1c60, 0x1c00, 0x7c30, 0x7c00,
    0x6c10, 0x6c00, 0x4c50, 0x3c00, 0x6454, 0x4c00, 0x0836, 0x4100,
    0x0077, 0x0000, 0x4136, 0x0800, 0x0201, 0x0201, 0x0205, 0x0200
};

static uint32_t convert_color(uint16_t color16) {
    uint8_t r = ((color16 >> 8) & 0xF) * 17;
    uint8_t g = ((color16 >> 4) & 0xF) * 17;
    uint8_t b = (color16 & 0xF) * 17;
    // RGBA8888
    return (r << 24) | (g << 16) | (b << 8) | 0xFF;
}

static void handle_hwi(DCPU_Hardware *hw, DCPU16 *cpu) {
    LEM1802 *lem = (LEM1802 *)hw;
    switch (cpu->reg[A]) {
        case 0: lem->vram_map = cpu->reg[B]; break;
        case 1: lem->font_map = cpu->reg[B]; break;
        case 2: lem->palette_map = cpu->reg[B]; break;
        case 3: lem->border_color_idx = cpu->reg[B] & 0xF; break;
        case 4:
            for (int i = 0; i < 256; i++) {
                cpu->ram[(uint16_t)(cpu->reg[B] + i)] = default_font[i];
            }
            cpu->cycles += 256;
            break;
        case 5:
            for (int i = 0; i < 16; i++) {
                cpu->ram[(uint16_t)(cpu->reg[B] + i)] = default_palette[i];
            }
            cpu->cycles += 16;
            break;
    }
}

void lem1802_init(LEM1802 *lem) {
    lem->base.hardware_id = LEM1802_HARDWARE_ID;
    lem->base.hardware_version = LEM1802_HARDWARE_VERSION;
    lem->base.manufacturer = LEM1802_MANUFACTURER;
    lem->base.handle_hwi = handle_hwi;

    lem->vram_map = 0;
    lem->font_map = 0;
    lem->palette_map = 0;
    lem->border_color_idx = 0;

    memset(lem->cache, 0, sizeof(lem->cache));
    memset(lem->pixels, 0, sizeof(lem->pixels));
    lem->dirty = true;
    lem->last_blink_time = SDL_GetTicks();
    lem->blink_state = false;
}

uint32_t lem1802_get_border_color(LEM1802 *lem, DCPU16 *cpu) {
    uint16_t border_rgb16 = (lem->palette_map)
        ? cpu->ram[(uint16_t)(lem->palette_map + lem->border_color_idx)]
        : default_palette[lem->border_color_idx];
    return convert_color(border_rgb16);
}

bool lem1802_update(LEM1802 *lem, DCPU16 *cpu, SDL_Texture *texture) {
    if (lem->vram_map == 0) return false;

    uint32_t now = SDL_GetTicks();
    if (now - lem->last_blink_time > 500) {
        lem->blink_state = !lem->blink_state;
        lem->last_blink_time = now;
    }

    bool screen_updated = false;

    for (int row = 0; row < LEM1802_ROWS; row++) {
        for (int col = 0; col < LEM1802_COLS; col++) {

            uint16_t vaddr = (uint16_t)(lem->vram_map + row * LEM1802_COLS + col);
            uint16_t cell = cpu->ram[vaddr];

            uint8_t char_idx = cell & 0x7F;
            bool blink       = (cell >> 7) & 0x01;
            uint8_t bg_idx   = (cell >> 8) & 0x0F;
            uint8_t fg_idx   = (cell >> 12) & 0x0F;

            uint16_t fg_rgb16 = (lem->palette_map) ? cpu->ram[(uint16_t)(lem->palette_map + fg_idx)] : default_palette[fg_idx];
            uint16_t bg_rgb16 = (lem->palette_map) ? cpu->ram[(uint16_t)(lem->palette_map + bg_idx)] : default_palette[bg_idx];

            uint32_t fg = convert_color(fg_rgb16);
            uint32_t bg = convert_color(bg_rgb16);

            uint32_t glyph = 0;
            if (!blink || lem->blink_state) {
                uint16_t lglyph = (lem->font_map) ? cpu->ram[(uint16_t)(lem->font_map + char_idx * 2)]     : default_font[char_idx * 2];
                uint16_t rglyph = (lem->font_map) ? cpu->ram[(uint16_t)(lem->font_map + char_idx * 2 + 1)] : default_font[char_idx * 2 + 1];
                glyph = ((uint32_t)lglyph << 16) | rglyph;
            }

            int tile_idx = row * LEM1802_COLS + col;
            if (lem->cache[tile_idx].glyph == glyph &&
                lem->cache[tile_idx].fg == fg &&
                lem->cache[tile_idx].bg == bg) {
                continue;
            }

            lem->cache[tile_idx].glyph = glyph;
            lem->cache[tile_idx].fg = fg;
            lem->cache[tile_idx].bg = bg;
            screen_updated = true;

            for (int dx = 0; dx < 4; dx++) {
                uint8_t column_data;
                if      (dx == 0) column_data = (glyph >> 24) & 0xFF;
                else if (dx == 1) column_data = (glyph >> 16) & 0xFF;
                else if (dx == 2) column_data = (glyph >> 8)  & 0xFF;
                else              column_data =  glyph        & 0xFF;

                for (int dy = 0; dy < 8; dy++) {
                    bool is_pixel_solid = (column_data >> dy) & 1;
                    int px = col * 4 + dx;
                    int py = row * 8 + dy;
                    lem->pixels[py * LEM1802_WIDTH + px] = is_pixel_solid ? fg : bg;
                }
            }
        }
    }

    if (screen_updated) {
        SDL_UpdateTexture(texture, NULL, lem->pixels, LEM1802_WIDTH * sizeof(uint32_t));
    }

    return screen_updated;
}