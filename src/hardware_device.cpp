#include "../include/hardware_device.h"
#include "../include/dcpu16.h"

#include <stdlib.h>

void connect_hardware(DCPU16 *cpu, DCPU_Hardware *new_device) {
    if (cpu->num_hardware >= DCPU_DEVICE_MAX ) {return;}
    size_t new_size = (cpu->num_hardware + 1) * sizeof(DCPU_Hardware *);
    DCPU_Hardware **temp_bus = realloc(cpu->bus, new_size);

    if (temp_bus == NULL) {
        return;
    }

    cpu->bus = temp_bus;
    cpu->bus[cpu->num_hardware] = new_device;
    cpu->num_hardware++;
}

void disconnect_hardware(DCPU16 *cpu, DCPU_Hardware *hardware) {
    if (cpu->bus != NULL) {
        free(cpu->bus);
        cpu->bus = NULL;
        cpu->num_hardware = 0;
    }
}