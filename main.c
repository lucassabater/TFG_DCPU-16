#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "include/hardware_device.h"
#include "include/emulation_utils.h"
#include "include/dcpu16.h"

int main(int argc, char *argv[]) {
    // 1. Basic Argument Check
    // If you run: ./emulator my_program.bin
    if (argc < 2) {
        printf("Usage: %s <rom_file.bin>\n", argv[0]);
        return 1;
    }

    // 2. CPU Initialization
    DCPU16 cpu;
    dcpu_init(&cpu);

    const char *rom_path = argv[1];
    //if (!load_rom(&cpu, rom_path)) {
    //    return 1;
    //}

    // 1. LIMPIEZA ABSOLUTA DE LA CPU
    memset(&cpu, 0, sizeof(DCPU16));

    // 2. PROGRAMA DE NOTCH DECODIFICADO (aaaaaa bbbbb ooooo)
    // Se han calculado los saltos para que :testsub esté en 0x18 y :crash en 0x1C
    uint16_t notch_v17_binario[] = {
        // --- Pruebas básicas ---
        0x7C01, 0x0030,          // [00-01] SET A, 0x30 (a: NW 0x1f, b: A 0x00, op: 0x01)
        0x7FC1, 0x0020, 0x1000,  // [02-04] SET [0x1000], 0x20 (Tu corrección: b=PC 0x1c)
        0x7803, 0x1000,          // [05-06] SUB A, [0x1000] (a: [NW] 0x1e, b: A 0x00, op: 0x03)
        0xC413,                  // [07]    IFN A, 0x10 (a: Lit 16 0x30, b: A 0x00, op: 0x13)
        0x7F81, 0x001C,          // [08-09] SET PC, crash (Salto a 0x1C)

        // --- Bucle (Loopy thing) ---
        0xAC61,                  // [0A]    SET I, 10 (a: Lit 10 0x2b, b: I 0x06, op: 0x01)
        0x7C01, 0x2000,          // [0B-0C] SET A, 0x2000
        0x2141, 0x2000,          // [0D-0E] SET [0x2000+I], [A] (Tu corrección: b=0x0a)
        0x88C3,                  // [0F]    SUB I, 1 (a: Lit 1 0x22, b: I 0x06, op: 0x03)
        0x84D3,                  // [10]    IFN I, 0 (a: Lit 0 0x21, b: I 0x06, op: 0x13)
        0x7F81, 0x000D,          // [11-12] SET PC, loop (Salto a 0x0D)

        // --- Subrutina ---
        0x9461,                  // [13]    SET X, 0x4 (a: Lit 4 0x25, b: X 0x03, op: 0x01)
        0x7C20, 0x0018,          // [14-15] JSR testsub (a: NW 0x1f, b: JSR 0x01, op: 0x00)
        0x7F81, 0x001C,          // [16-17] SET PC, crash (Salto a 0x1C)

        // --- :testsub (Dirección 0x18) ---
        0x946F,                  // [18]    SHL X, 4 (a: Lit 4 0x25, b: X 0x03, op: 0x0f)
        0x6381,                  // [19]    SET PC, POP (a: POP 0x18, b: PC 0x1c, op: 0x01)
        0x0000,                  // [1A]    Padding (Alineación)
        0x0000,                  // [1B]    Padding (Alineación)

        // --- :crash (Dirección 0x1C) ---
        0xEEE0                   // [1C]    WORD 0xeee0
    };

    // 3. INYECCIÓN EN RAM
    for (size_t i = 0; i < (sizeof(notch_v17_binario) / sizeof(uint16_t)); i++) {
        cpu.ram[i] = notch_v17_binario[i];
    }

    // 5. Execution Loop
    printf("Starting emulation...\n\n");

    bool running = true;
    int instructions_to_run = 20;

    for (int i = 0; i < instructions_to_run && running; i++) {
        printf("--- Step %d ---\n", i + 1);

        dcpu_step(&cpu);

        cpu_dump(&cpu);

    }

    printf("Emulation finished.\n");
    return 0;
}
