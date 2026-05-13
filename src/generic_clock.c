#include "generic_clock.h"
#include "dcpu16.h"

static void handle_hwi(DCPU_Hardware *hw, DCPU16 *cpu) {
    GenericClock *clock = (GenericClock *)hw;

    switch (cpu->reg[0]) {
        case 0: // SET_TICK_RATE
            clock->interval = cpu->reg[1];
            clock->cycle_accumulator = 0;
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
    clock->cycle_accumulator = 0;
}

void clock_tick(GenericClock *clock, DCPU16 *cpu, uint32_t cycles_executed) {
    if (clock->interval == 0) return;
    clock->cycle_accumulator += cycles_executed;

    uint32_t cycles_per_tick = (uint32_t)((CLOCK_HZ * clock->interval) / 60);

    if (clock->cycle_accumulator >= cycles_per_tick) {
        clock->ticks++;
        clock->cycle_accumulator -= cycles_per_tick;
        if (clock->int_message != 0) {
            interrupt_enqueue(cpu, clock->int_message);
        }
    }
}