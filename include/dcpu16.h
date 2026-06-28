/**
 * @file dcpu16.h
 * @brief Core CPU definitions and state structures for the DCPU-16.
 */

#ifndef DCPU16_H
#define DCPU16_H

#include <stdbool.h>
#include <stdint.h>

#define DCPU_REG_N 8
#define DCPU_RAM_SIZE 65536
#define DCPU_DEVICE_MAX 65535
#define DCPU_INTERRUPTQ_SIZE 256

#define GET_OPCODE(instr) ((instr) & 0x1F)
#define GET_B(instr)      (((instr) >> 5) & 0x1F)
#define GET_A(instr)      (((instr) >> 10) & 0x3F)

#define FBIT_MASK 0xFFFF

// Forward declaration to resolve circular dependency
typedef struct DCPU_Hardware DCPU_Hardware;

typedef struct DCPU16 {
    uint16_t ram[DCPU_RAM_SIZE];
    uint16_t reg[DCPU_REG_N];
    uint16_t pc;
    uint16_t sp;
    uint16_t ex;
    uint16_t ia;

    uint16_t interruptq[DCPU_INTERRUPTQ_SIZE];
    bool interrupt_queueing;
    uint8_t iq_head;
    uint8_t iq_tail;
    uint16_t iq_count;
    bool is_on_fire;

    DCPU_Hardware **bus;
    uint16_t num_hardware;

    unsigned long cycles;
    bool halted;
} DCPU16;

typedef enum {
    OP_SPECIAL = 0x00, OP_SET = 0x01, OP_ADD = 0x02, OP_SUB = 0x03,
    OP_MUL = 0x04, OP_MLI = 0x05, OP_DIV = 0x06, OP_DVI = 0x07,
    OP_MOD = 0x08, OP_MDI = 0x09, OP_AND = 0x0a, OP_BOR = 0x0b,
    OP_XOR = 0x0c, OP_SHR = 0x0d, OP_ASR = 0x0e, OP_SHL = 0x0f,
    OP_IFB = 0x10, OP_IFC = 0x11, OP_IFE = 0x12, OP_IFN = 0x13,
    OP_IFG = 0x14, OP_IFA = 0x15, OP_IFL = 0x16, OP_IFU = 0x17,
    OP_ADX = 0x1a, OP_SBX = 0x1b, OP_STI = 0x1e, OP_STD = 0x1f
} opcode_e;

typedef enum {
    SOP_JSR = 0x01, SOP_INT = 0x08, SOP_IAG = 0x09,
    SOP_IAS = 0x0a, SOP_RFI = 0x0b, SOP_IAQ = 0x0c, SOP_HWN = 0x10,
    SOP_HWQ = 0x11, SOP_HWI = 0x12
} specop_e;

typedef enum {
    A = 0x00, B, C, X, Y, Z, I, J
} registers_e;

typedef enum {
    REG_START = 0x00, REG_END = 0x07,
    PTR_REG = 0x08, PTR_REG_END = 0x0F,
    PTR_REG_NW = 0x10, PTR_REG_NW_END = 0x17,
    PUSH_POP = 0x18, PEEK = 0x19, PICK = 0x1a,
    SP = 0x1b, PC = 0x1c, EX = 0x1d, PTR_NW = 0x1e, NW = 0x1f,
    LITERAL_START = 0x20, LITERAL_END = 0x3F,
} operand_value_e;

// Public API
void dcpu_init(DCPU16 *cpu);
uint32_t dcpu_step(DCPU16 *cpu);
void dcpu_interrupt_enqueue(DCPU16 *cpu, uint16_t message);
void dcpu_add_hardware(DCPU16 *cpu, DCPU_Hardware *hardware);


#endif // DCPU16_H