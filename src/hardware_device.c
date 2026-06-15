/**
* @file hardware_device.c
 * @brief Hardware bus management and device connection for the DCPU-16 emulator.
 */

#include "hardware_device.h"
#include "dcpu16.h"
#include <stdlib.h>

/**
 * @brief Connects a new hardware device to the CPU bus dynamically.
 * * @param cpu Pointer to the DCPU16 instance.
 * @param new_device Pointer to the hardware device interface to connect.
 */
void dcpu_connect_hardware(DCPU16 *cpu, DCPU_Hardware *new_device) {
    if (cpu == NULL || new_device == NULL) return;

    if (cpu->num_hardware >= DCPU_DEVICE_MAX) {
        return;
    }

    size_t new_size = (cpu->num_hardware + 1) * sizeof(DCPU_Hardware *);
    DCPU_Hardware **temp_bus = realloc(cpu->bus, new_size);

    if (temp_bus == NULL) {
        return;
    }

    cpu->bus = temp_bus;
    cpu->bus[cpu->num_hardware] = new_device;
    cpu->num_hardware++;
}