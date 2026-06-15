/**
* @file hardware_device.h
 * @brief Universal hardware bus interface for DCPU-16 peripherals.
 */

#ifndef HARDWARE_DEVICE_H
#define HARDWARE_DEVICE_H

#include <stdint.h>

// Forward declaration
typedef struct DCPU16 DCPU16;

typedef struct DCPU_Hardware {
    uint32_t hardware_id;
    uint16_t hardware_version;
    uint32_t manufacturer;

    void (*handle_hwi)(struct DCPU_Hardware *hw, DCPU16 *cpu);
} DCPU_Hardware;

void dcpu_connect_hardware(DCPU16 *cpu, DCPU_Hardware *hardware);
void dcpu_disconnect_all_hardware(DCPU16 *cpu);

#endif // HARDWARE_DEVICE_H