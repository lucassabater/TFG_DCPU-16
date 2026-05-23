
#ifndef UNTITLED_EMULATIO_UTILS_H
#define UNTITLED_EMULATIO_UTILS_H

#include "hardware_device.h"
#include "dcpu16.h"

#include <stdio.h>
#include <stdint.h>

bool load_rom(DCPU16 *cpu, const char *file_path) {
       FILE *file = fopen(file_path, "r");
       if (file == NULL) {
              printf("ERROR: No se pudo abrir el archivo: %s\n", file_path);
              return false;
       }

       size_t words_read = 0;
       uint16_t value;

       // El formato "%hx," le dice: busca un hexadecimal y luego ignora una coma
       // Añadimos un espacio delante " %hx" para que ignore saltos de línea/espacios
       while (fscanf(file, " %4hx ,", &value) >= 1 && words_read < 65536) {
              cpu->ram[words_read] = value;
              printf("Leído [%04zX]: %04X\n", words_read, cpu->ram[words_read]);
              words_read++;
       }

       fclose(file);

       if (words_read == 0) {
              printf("ERROR: No se encontraron datos válidos (revisa el formato).\n");
              return false;
       }

       printf("SUCCESS: ROM cargada con %zu palabras.\n", words_read);
       return true;
}

bool load_bin(DCPU16 *cpu, const char *filename) {
       // IMPORTANTE: "rb" significa Read Binary. Evita que Windows corrompa los saltos de línea.
       FILE *f = fopen(filename, "rb");
       if (!f) return false;

       uint16_t addr = 0;
       uint8_t bytes[2];

       // Leemos el archivo de 2 en 2 bytes hasta llegar al final o llenar la RAM
       while (fread(bytes, 1, 2, f) == 2 && addr < 0x10000) {
              // Reconstruimos la palabra de 16 bits (Big-Endian -> Ordenador PC)
              // El primer byte leído se mueve a la izquierda, el segundo se suma.
              uint16_t word = (bytes[0] << 8) | bytes[1];

              cpu->ram[addr++] = word;
       }

       fclose(f);
       printf("BIN ROM LOAD: Leidas %d palabras exactas.\n", addr);

       // Mostramos las 5 primeras posiciones para confirmar
       printf("\n--- INSPECCION DE MEMORIA INICIAL (BIN) ---\n");
       for (int i = 0; i < 5; i++) {
              printf("RAM[%04X] = %04X\n", i, cpu->ram[i]);
       }
       printf("-------------------------------------------\n\n");

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