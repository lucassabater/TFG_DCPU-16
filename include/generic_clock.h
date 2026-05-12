#ifndef UNTITLED_GENERIC_CLOCK_H
#define UNTITLED_GENERIC_CLOCK_H

#include <stdint.h>
#include "hardware_device.h"
#include "dcpu16.h"

#define CLOCK_HARDWARE_ID      0x12d0b402
#define CLOCK_HARDWARE_VERSION 0x0001
#define CLOCK_MANUFACTURER     0x00000000

typedef struct {
    DCPU_Hardware base;
    uint16_t interval;
    uint16_t int_message;
    uint16_t ticks;

    uint64_t next_tick_time;
    uint32_t ms_per_tick;
} GenericClock;

void clock_init(GenericClock *clock);
void clock_tick(GenericClock *clock, DCPU16 *cpu);

#endif