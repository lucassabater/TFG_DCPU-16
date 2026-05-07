#ifndef HARDWARE_DEVICE_H
#define HARDWARE_DEVICE_H

#include <stdint.h>

typedef struct DCPU16 DCPU16;

typedef struct DCPU_Hardware {
    uint32_t hardware_id;
    uint16_t hardware_version;
    uint32_t manufacturer;

    void (*handle_hwi)(struct DCPU_Hardware *hw, DCPU16 *cpu);
} DCPU_Hardware;

void connect_hardware(DCPU16 *cpu, DCPU_Hardware *hardware);
void disconnect_hardware(DCPU16 *cpu, DCPU_Hardware *hardware);
#endif