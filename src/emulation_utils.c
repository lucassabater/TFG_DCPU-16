#include "emulation_utils.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define LINE_BUFFER_SIZE 512

bool load_rom(DCPU16 *cpu, const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        printf("ERROR: No se pudo abrir el archivo de texto: %s\n", file_path);
        return false;
    }

    size_t words_read = 0;
    char line[LINE_BUFFER_SIZE];

    while (fgets(line, sizeof(line), file) && words_read < DCPU_RAM_SIZE) {
        char *ptr = line;
        char *endptr;

        while (words_read < DCPU_RAM_SIZE) {
            unsigned long val = strtoul(ptr, &endptr, 16);

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
        printf("ERROR: No se encontraron datos válidos. ¿Seguro que es un archivo de texto con hex?\n");
        return false;
    }

    printf("SUCCESS: ROM de TEXTO cargada con %zu palabras.\n", words_read);
    return true;
}


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