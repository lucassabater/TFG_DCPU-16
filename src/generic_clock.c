#include "../include/generic_clock.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

static uint64_t current_time_ms() {
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + (ts.tv_nsec / 1000000));
#endif
}

static void handle_hwi(DCPU_Hardware *hw, DCPU16 *cpu) {
    GenericClock *clock = (GenericClock *)hw;

    switch (cpu->reg[0]) {
        case 0: // SET_TICK_RATE
            clock->interval = cpu->reg[1];
            if (clock->interval == 0) {
                clock->next_tick_time = 0;
            } else {
                clock->ms_per_tick = (1000 * clock->interval) / 60;
                clock->next_tick_time = current_time_ms() + clock->ms_per_tick;
            }
            clock->ticks = 0;
            break;

        case 1: // GET_TICKS
            cpu->reg[2] = clock->ticks;
            break;

        case 2: // SET_INTERRUPT_MESSAGE
            clock->int_message = cpu->reg[1];
            break;

        default: break;
    }
}

void clock_init(GenericClock *clock) {
    clock->base.hardware_id = CLOCK_HARDWARE_ID;
    clock->base.hardware_version = CLOCK_HARDWARE_VERSION;
    clock->base.manufacturer = CLOCK_MANUFACTURER;
    clock->base.handle_hwi = handle_hwi;

    clock->interval = 0;
    clock->ticks = 0;
    clock->int_message = 0;
    clock->next_tick_time = 0;
}

void clock_tick(GenericClock *clock, DCPU16 *cpu) {
    if (clock->interval == 0) return;

    uint64_t now = current_time_ms();

    if (now >= clock->next_tick_time) {
        clock->ticks++;

        if (clock->int_message != 0) {
            interrupt_enqueue(cpu, clock->int_message);
        }

        clock->next_tick_time += clock->ms_per_tick;
    }
}