#ifndef UNTITLED_CPU_H
#define UNTITLED_CPU_H

#include <stdbool.h>
#define word_length 16

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint16_t ram[65536];   // 64K palabras de memoria
    uint16_t reg[8];       // Registros A, B, C, X, Y, Z, I, J
    uint16_t pc;
    uint16_t sp;
    uint16_t ex;
    uint16_t ia;

    bool is_on_fire;
    unsigned long cycles;
} DCPU16;

// --- Opcodes Básicos (5 bits) ---
typedef enum {
    OP_SPECIAL = 0x00,
    OP_SET = 0x01,
    OP_ADD = 0x02,
    OP_SUB = 0x03,
    OP_MUL = 0x04,
    OP_MLI = 0x05,
    OP_DIV = 0x06,
    OP_DVI = 0x07,
    OP_MOD = 0x08,
    OP_MDI = 0x09,
    OP_AND = 0x0a,
    OP_BOR = 0x0b,
    OP_XOR = 0x0c,
    OP_SHR = 0x0d,
    OP_ASR = 0x0e,
    OP_SHL = 0x0f,
    OP_IFB = 0x10,
    OP_IFC = 0x11,
    OP_IFE = 0x12,
    OP_IFN = 0x13,
    OP_IFG = 0x14,
    OP_IFA = 0x15,
    OP_IFL = 0x16,
    OP_IFU = 0x17,
    OP_ADX = 0x1a,
    OP_SBX = 0x1b,
    OP_STI = 0x1e,
    OP_STD = 0x1f
} opcode_e;


// --- Opcodes Especiales (5 bits en el medio de la instrucción) ---
typedef enum {
    SOP_JSR = 0x01,
    SOP_INT = 0x08,
    SOP_IAG = 0x09,
    SOP_IAS = 0x0a,
    SOP_RFI = 0x0b,
    SOP_IAQ = 0x0c,
    SOP_HWN = 0x10,
    SOP_HWQ = 0x11,
    SOP_HWI = 0x12

} specop_e;


// --- Tipos de Operandos / Valores (6 bits para 'a', 5 bits para 'b') ---
// Para los registros y punteros, en vez de definir 8 constantes para cada uno,
// es más eficiente definir el inicio del rango.
typedef enum {
    REG_START         = 0x00, // 0x00 - 0x07: Registros (A, B, C, X, Y, Z, I, J)
    PTR_REG     = 0x08, // 0x08 - 0x0F: [Registro]
    PTR_REG_NW  = 0x10, // 0x10 - 0x17: [Registro + next word]

    PUSH_POP          = 0x18, // PUSH (si es b) / POP (si es a)
    PEEK              = 0x19, // [SP]
    PICK              = 0x1a, // [SP + next word]
    SP                = 0x1b, // SP
    PC                = 0x1c, // PC
    EX                = 0x1d, // EX
    PTR_NW            = 0x1e, // [next word]
    NW                = 0x1f, // next word (literal)

    LITERAL_START     = 0x20  // 0x20 - 0x3F: Literal corto (0xffff - 0x1e)
} operand_value_e;

typedef enum {
    REG_A = 0,
    REG_B = 1,
    REG_C = 2,
    REG_X = 3,
    REG_Y = 4,
    REG_Z = 5,
    REG_I = 6,
    REG_J = 7
} register_e;

void dcpu_init(DCPU16 *cpu);
void cpu_parse(DCPU16 *cpu);
uint16_t* operand_val(DCPU16 *cpu, uint_fast8_t number, bool is_a);
void specop_parse(DCPU16 *cpu, opcode_e opcode);
static void skip_instruction(DCPU16 *cpu);
#endif