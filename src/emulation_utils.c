/**
 * @file emulation_utils.c
 * @brief Utility functions for the DCPU-16 emulator (ROM loading, debugging).
 */

#include "emulation_utils.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define LINE_BUFFER_SIZE 512

/**
 * @brief Loads a hex-encoded text ROM into the DCPU-16 RAM.
 *
 * @param cpu Pointer to the DCPU16 instance.
 * @param file_path Path to the text-based ROM file.
 * @return true on success, false on failure.
 */
bool load_rom(DCPU16 *cpu, const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        fprintf(stderr, "ERROR: Failed to open ROM text file: %s\n", file_path);
        return false;
    }

    size_t words_read = 0;
    char line[LINE_BUFFER_SIZE];

    while (fgets(line, sizeof(line), file) && words_read < DCPU_RAM_SIZE) {
        char *ptr = line;
        char *endptr;

        while (words_read < DCPU_RAM_SIZE) {
            unsigned long val = strtoul(ptr, &endptr, 16);

            // Skip non-hexadecimal characters safely
            if (ptr == endptr) {
                if (*ptr == '\0') break;
                ptr++;
                continue;
            }

            cpu->ram[words_read] = (uint16_t)val;
            words_read++;
            ptr = endptr;
        }
    }

    fclose(file);

    if (words_read == 0) {
        fprintf(stderr, "ERROR: No valid data found. Ensure the file contains valid hex values.\n");
        return false;
    }

    printf("SUCCESS: Text ROM loaded with %zu words.\n", words_read);
    return true;
}

/**
 * @brief Dumps the current CPU state to standard output for debugging.
 *
 * @param cpu Pointer to the DCPU16 instance.
 */
void cpu_dump(const DCPU16 *cpu) {
    printf("=== DCPU-16 STATE DUMP ===\n");

    printf("PC: %04X | SP: %04X | EX: %04X | IA: %04X\n",
           cpu->pc, cpu->sp, cpu->ex, cpu->ia);
    printf("Total Cycles: %u\n", cpu->cycles);

    printf("--- General Purpose Registers ---\n");
    printf("A: %04X | B: %04X | C: %04X | X: %04X\n",
           cpu->reg[A], cpu->reg[B], cpu->reg[C], cpu->reg[X]);
    printf("Y: %04X | Z: %04X | I: %04X | J: %04X\n",
           cpu->reg[Y], cpu->reg[Z], cpu->reg[I], cpu->reg[J]);

    printf("--- Hardware Bus ---\n");
    printf("Devices connected: %u\n", cpu->num_hardware);
    printf("==========================\n\n");
}