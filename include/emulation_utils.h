#ifndef EMULATION_UTILS_H
#define EMULATION_UTILS_H

#include <stdbool.h>
#include "dcpu16.h"

// Forward declaration of the DCPU16 struct to avoid circular dependencies
// This tells the compiler "DCPU16 exists" without needing to include cpu.h here.

/**
 * Loads a binary file into the CPU's RAM.
 * Automatically performs byte-swapping to handle Big-Endian ROMs on Little-Endian PCs.
 * * @param cpu Pointer to the DCPU16 instance.
 * @param file_path Path to the .bin file.
 * @return true if successful, false otherwise.
 */
bool load_rom(DCPU16 *cpu, const char *file_path);

/**
 * Prints a complete snapshot of the CPU's internal state to the console.
 * Includes registers (A, B, C, X, Y, Z, I, J), PC, SP, EX, and cycle count.
 * * @param cpu Pointer to the DCPU16 instance (read-only).
 */
void cpu_dump(const DCPU16 *cpu);

#endif