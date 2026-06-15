/**
 * @file generic_clock.c
 * @brief Implementation of the DCPU-16 Generic Clock (Hardware ID 0x12d0b402).
 */

#include "generic_clock.h"
#include "dcpu16.h"

/**
 * @brief Processes Hardware Interrupts (HWI) sent by the CPU to the clock.
 */
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

        default:
            break;
    }
}

/**
 * @brief Initializes the clock peripheral state and standard DCPU metadata.
 * * @param clock Pointer to the GenericClock instance.
 */
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

/**
 * @brief Advances the internal clock state based on executed CPU cycles.
 * * @param clock Pointer to the GenericClock instance.
 * @param cpu Pointer to the DCPU16 instance (for interrupt queueing).
 * @param cycles_executed Number of CPU cycles consumed in the last step.
 */
void clock_tick(GenericClock *clock, DCPU16 *cpu, uint32_t cycles_executed) {
    // A tick interval of 0 means the clock is disabled
    if (clock->interval == 0) return;

    clock->cycle_accumulator += cycles_executed;

    // Calculate required cycles per tick using 64-bit math to prevent overflow
    uint32_t cycles_per_tick = (uint32_t)(((uint64_t)CLOCK_HZ * clock->interval) / CLOCK_BASE_HZ);
    if (cycles_per_tick == 0) {
        cycles_per_tick = 1;
    }

    // Trigger ticks and enqueue interrupts if accumulator threshold is met
    while (clock->cycle_accumulator >= cycles_per_tick) {
        clock->ticks++;
        clock->cycle_accumulator -= cycles_per_tick;

        if (clock->int_message != 0) {
            dcpu_interrupt_enqueue(cpu, clock->int_message);
        }
    }
}