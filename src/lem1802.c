#include "lem1802.h"
#include "lem_data.h"
#include "dcpu16.h"

static uint32_t convert_color(uint16_t color16) {
    uint8_t r = ((color16 >> 8) & 0x0F) * 0x11;
    uint8_t g = ((color16 >> 4) & 0x0F) * 0x11;
    uint8_t b = (color16 & 0x0F) * 0x11;
    return (r << 24) | (g << 16) | (b << 8) | 0xFF;
}

static void handle_hwi(DCPU_Hardware *hw, DCPU16 *cpu) {
    LEM1802 *lem = (LEM1802 *)hw;

    switch (cpu->reg[A]) {
        case 0: lem->vram_map = cpu->reg[B]; break;
        case 1: lem->font_map = cpu->reg[B]; break;
        case 2: lem->palette_map = cpu->reg[B]; break;
        case 3: lem->border_color_idx = cpu->reg[B] & 0x0F; break;

        case 4: // DUMP_FONT
            for (int i = 0; i < LEM1802_FONT_SIZE; i++) {
                cpu->ram[(uint16_t)(cpu->reg[B] + i)] = lem1802_default_font[i];
            }
            cpu->cycles += LEM1802_FONT_SIZE;
            break;

        case 5: // DUMP_PALETTE
            for (int i = 0; i < LEM1802_PALETTE_SIZE; i++) {
                cpu->ram[(uint16_t)(cpu->reg[B] + i)] = lem1802_default_palette[i];
            }
            cpu->cycles += LEM1802_PALETTE_SIZE;
            break;

        default: break;
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
    uint16_t border_rgb16 = (lem->palette_map != 0)
        ? cpu->ram[(uint16_t)(lem->palette_map + lem->border_color_idx)]
        : lem1802_default_palette[lem->border_color_idx];

    return convert_color(border_rgb16);
}

bool lem1802_update(LEM1802 *lem, DCPU16 *cpu, SDL_Texture *texture) {
    if (lem->vram_map == 0) return false;

    uint32_t now = SDL_GetTicks();
    bool blink_changed = false;

    if (now - lem->last_blink_time > LEM1802_BLINK_RATE_MS) {
        lem->blink_state = !lem->blink_state;
        lem->last_blink_time = now;
        blink_changed = true;
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

            uint16_t lglyph = (lem->font_map != 0) ? cpu->ram[(uint16_t)(lem->font_map + char_idx * 2)]     : lem1802_default_font[char_idx * 2];
            uint16_t rglyph = (lem->font_map != 0) ? cpu->ram[(uint16_t)(lem->font_map + char_idx * 2 + 1)] : lem1802_default_font[char_idx * 2 + 1];
            uint32_t base_glyph = ((uint32_t)lglyph << 16) | rglyph;

            uint16_t fg_rgb16 = (lem->palette_map != 0) ? cpu->ram[(uint16_t)(lem->palette_map + fg_idx)] : lem1802_default_palette[fg_idx];
            uint16_t bg_rgb16 = (lem->palette_map != 0) ? cpu->ram[(uint16_t)(lem->palette_map + bg_idx)] : lem1802_default_palette[bg_idx];

            uint32_t fg = convert_color(fg_rgb16);
            uint32_t bg = convert_color(bg_rgb16);

            uint32_t visual_glyph = (blink && !lem->blink_state) ? 0x00000000 : base_glyph;

            int tile_idx = row * LEM1802_COLS + col;
            if (lem->cache[tile_idx].glyph == visual_glyph &&
                lem->cache[tile_idx].fg == fg &&
                lem->cache[tile_idx].bg == bg &&
                !blink_changed) {
                continue;
            }

            lem->cache[tile_idx].glyph = visual_glyph;
            lem->cache[tile_idx].fg = fg;
            lem->cache[tile_idx].bg = bg;
            screen_updated = true;

            for (int dx = 0; dx < LEM1802_COL_FACTOR; dx++) {
                uint8_t column_data = (visual_glyph >> (24 - (dx * 8))) & 0xFF;

                for (int dy = 0; dy < LEM1802_ROW_FACTOR; dy++) {
                    bool is_pixel_solid = (column_data >> dy) & 1;

                    int px = col * LEM1802_COL_FACTOR + dx;
                    int py = row * LEM1802_ROW_FACTOR + dy;

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