#ifndef UNTITLED_GENERIC_CLOCK_H
#define UNTITLED_GENERIC_CLOCK_H

#include <stdint.h>
#include "hardware_device.h"

#define CLOCK_HZ              100000
#define CLOCK_HARDWARE_ID      0x12d0b402
#define CLOCK_HARDWARE_VERSION 0x0001
#define CLOCK_MANUFACTURER     0x00000000

typedef struct {
    DCPU_Hardware base;
    uint16_t interval;
    uint16_t int_message;
    uint16_t ticks;
    uint32_t cycle_accumulator;
} GenericClock;

void clock_init(GenericClock *clock);
void clock_tick(GenericClock *clock, DCPU16 *cpu, uint32_t cycles);

#endif