/**
* @file emulation_utils.h
 * @brief Utility functions for ROM loading and debugging.
 */

#ifndef EMULATION_UTILS_H
#define EMULATION_UTILS_H

#include <stdbool.h>
#include "dcpu16.h"

/**
 * @brief Loads a hex-encoded text ROM into the CPU's RAM.
 * * @param cpu Pointer to the DCPU16 instance.
 * @param file_path Path to the text ROM file.
 * @return true if successful, false otherwise.
 */
bool load_rom(DCPU16 *cpu, const char *file_path);

/**
 * @brief Dumps a complete snapshot of the CPU's internal state to stdout.
 * * @param cpu Pointer to the DCPU16 instance (read-only).
 */
void cpu_dump(const DCPU16 *cpu);

#endif // EMULATION_UTILS_H