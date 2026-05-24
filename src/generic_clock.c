#include "generic_clock.h"
#include "dcpu16.h"

static void handle_hwi(DCPU_Hardware *hw, DCPU16 *cpu) {
    GenericClock *clock = (GenericClock *)hw;

    switch (cpu->reg[A]) {
        case 0: // SET_TICK_RATE
            clock->interval = cpu->reg[B];
            clock->cycle_accumulator = 0;
            clock->ticks = 0;
            break;

        case 1: // GET_TICKS
            cpu->reg[C] = clock->ticks;
            break;

        case 2: // SET_INTERRUPT_MESSAGE
            clock->int_message = cpu->reg[B];
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
    clock->cycle_accumulator = 0;
}

void clock_tick(GenericClock *clock, DCPU16 *cpu, uint32_t cycles_executed) {
    if (clock->interval == 0) return;
    clock->cycle_accumulator += cycles_executed;

    uint32_t cycles_per_tick = (uint32_t)(((uint64_t)CLOCK_HZ * clock->interval) / CLOCK_BASE_HZ);
    if (cycles_per_tick == 0) cycles_per_tick = 1;

    while (clock->cycle_accumulator >= cycles_per_tick) {
        clock->ticks++;
        clock->cycle_accumulator -= cycles_per_tick;
        if (clock->int_message != 0) {
            dcpu_interrupt_enqueue(cpu, clock->int_message);
        }
    }
}