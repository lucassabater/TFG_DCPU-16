
#ifndef UNTITLED_EMULATIO_UTILS_H
#define UNTITLED_EMULATIO_UTILS_H

#include "../include/hardware_device.h"
#include "../include/dcpu16.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

bool load_rom(DCPU16 *cpu, const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        printf("ERROR: Could not open the ROM: %s\n", file_path);
        return false;
    }

    size_t words_read = fread(cpu->ram, sizeof(uint16_t), 65536, file);

    fclose(file);

    for (size_t i = 0; i < words_read; i++) {
        uint16_t value = cpu->ram[i];
        cpu->ram[i] = (value >> 8) | (value << 8);
    }

    printf("SUCCESS: ROM loaded. %zu words written to RAM.\n", words_read);
    return true;
}

void cpu_dump(const DCPU16 *cpu) {
    printf("=== DCPU-16 STATE DUMP ===\n");

    printf("PC: %04X | SP: %04X | EX: %04X | IA: %04X\n",
           cpu->pc, cpu->sp, cpu->ex, cpu->ia);
    printf("Total Cycles: %u\n", cpu->cycles);

    printf("--- General Purpose Registers ---\n");
    printf("A: %04X | B: %04X | C: %04X | X: %04X\n",
           cpu->reg[0], cpu->reg[1], cpu->reg[2], cpu->reg[3]);
    printf("Y: %04X | Z: %04X | I: %04X | J: %04X\n",
           cpu->reg[4], cpu->reg[5], cpu->reg[6], cpu->reg[7]);

    printf("--- Hardware Bus ---\n");
    printf("Devices connected: %u\n", cpu->num_hardware);
    printf("==========================\n\n");
}
#endif