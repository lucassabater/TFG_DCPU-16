#include <stdio.h>

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
    if (!load_rom(&cpu, rom_path)) {
        return 1;
    }

    // 5. Execution Loop
    printf("Starting emulation...\n\n");


    while (!cpu.halted){
        dcpu_step(&cpu);

        cpu_dump(&cpu);
    }

    printf("Emulation finished.\n");
    return 0;
}
