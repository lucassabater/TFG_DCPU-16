#include "dcpu16.h"
#include "hardware_device.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static uint16_t dummy_literal = 0;

void dcpu_init(DCPU16 *cpu){
    memset(cpu->ram, 0, sizeof(cpu->ram));

    memset(cpu->reg, 0, sizeof(cpu->reg));
    cpu->pc = 0x0000;
    cpu->sp = 0x0000;
    cpu->ia = 0x0000;
    cpu->ex = 0x0000;
    cpu->cycles = 0;

    memset(cpu->interruptq, 0, sizeof(cpu->interruptq));
    cpu->iq_head = 0;
    cpu->iq_tail = 0;
    cpu->iq_count = 0;
    cpu->interrupt_enabled = true;
    cpu->is_on_fire = false;

    cpu->num_hardware = 0;
    cpu->bus = NULL;
    cpu->halted = false;
}

uint32_t dcpu_step(DCPU16 *cpu) {

    interrupt_dequeue(cpu);

    uint32_t initial_cycles = cpu->cycles;

    const uint16_t instr = cpu->ram[cpu->pc++];
    const uint8_t opcode = instr & 0x1F;

    uint16_t *ptr_a = operand_val(cpu, (instr >> 10) & 0x3F, true);
    uint16_t a = *ptr_a;

    if (opcode == OP_SPECIAL) {
        specop_exec(cpu, ptr_a, a, (instr >> 5) & 0x1F);
    } else {
        uint16_t *ptr_b = operand_val(cpu, (instr >> 5) & 0x1F, false);
        uint16_t b = *ptr_b;

        switch (opcode) {
            case OP_SET:
                *ptr_b = a;
                cpu->cycles++;
                break;

            case OP_ADD: {
                const uint32_t rest = b + a;
                *ptr_b = rest & 0xffff;
                cpu->ex = (rest > 0xFFFF) ? 0x0001 : 0x0000;
                cpu->cycles += 2;
                break;
            }

            case OP_SUB: {
                const uint32_t rest = b - a;
                *ptr_b = rest & 0xFFFF;
                cpu->ex = (b < a) ? 0xFFFF : 0x0000;
                cpu->cycles += 2;
                break;
            }

            case OP_MUL: {
                const uint32_t rest = (uint32_t)b * (uint32_t)a;
                *ptr_b = rest & 0xFFFF;
                cpu->ex = ((rest)>>16)&0xffff;
                cpu->cycles += 2;
                break;
            }

            case OP_MLI: {
                const int32_t rest = (int32_t)(int16_t)b * (int32_t)(int16_t)a;
                *ptr_b = rest & 0xFFFF;
                cpu->ex = rest>>16 & 0xffff;
                cpu->cycles += 2;
                break;
            }

            case OP_DIV: {
                if (a==0) {
                    *ptr_b = 0;
                    cpu->ex = 0;
                    cpu->cycles += 3;
                    break;
                }
                *ptr_b = b / a;
                cpu->ex = ((uint32_t)b << 16) / a & 0xffff;
                cpu->cycles += 3;
                break;
            }

            case OP_DVI:{
                if (a==0) {
                    *ptr_b = 0;
                    cpu->ex = 0;
                    cpu->cycles += 3;
                    break;
                }
                *ptr_b = (int16_t)b / (int16_t)a;
                cpu->ex = (((int32_t)(int16_t)b << 16) / (int16_t)a) & 0xFFFF;
                cpu->cycles += 3;
                break;
            }

            case OP_MOD: {
                if (a==0) {
                    *ptr_b = 0;
                } else {
                    *ptr_b = b % a;
                }
                cpu->cycles += 3;
                break;
            }

            case OP_MDI:{
                if (a==0) {
                    *ptr_b = 0;
                } else {
                    *ptr_b = (int16_t)b % (int16_t)a;
                }
                cpu->cycles += 3;
                break;
            }

            case OP_AND: {
                *ptr_b = a & b;
                cpu->cycles++;
                break;
            }

            case OP_BOR: {
                *ptr_b = a | b;
                cpu->cycles++;
                break;
            }

            case OP_XOR:{
                *ptr_b = a ^ b;
                cpu->cycles++;
                break;
            }

            case OP_SHR:{
                if (a >= 16) {
                    *ptr_b = 0;
                    cpu->ex = (a >= 32) ? 0 : (b >> (a - 16)) & 0xFFFF;
                } else {
                    *ptr_b = (b >> a) & 0xFFFF;
                    cpu->ex = (((uint32_t)b << 16) >> a) & 0xFFFF;
                }
                cpu->cycles++;
                break;
            }

            case OP_ASR: {
                int16_t signed_b = (int16_t)b;
                if (a >= 16) {
                    *ptr_b = (signed_b < 0) ? 0xFFFF : 0x0000;
                    cpu->ex = (signed_b < 0) ? 0xFFFF : 0x0000;
                } else {
                    *ptr_b = (signed_b >> a) & 0xFFFF;
                    cpu->ex = (((int32_t)signed_b << 16) >> a) & 0xFFFF;
                }
                cpu->cycles++;
                break;
            }

            case OP_SHL: {
                if (a >= 16) {
                    *ptr_b = 0;
                    cpu->ex = (a >= 32) ? 0 : (b << (a - 16)) & 0xFFFF;
                } else {
                    *ptr_b = (b << a) & 0xFFFF;
                    cpu->ex = (((uint32_t)b << a) >> 16) & 0xFFFF;
                }
                cpu->cycles += 1;
                break;
            }

            case OP_IFB: {
                if ((b & a) == 0) {
                    skip_instruction(cpu);
                    cpu->cycles++;
                }
                cpu->cycles += 2;
                break;
            }

            case OP_IFC: {
                if ((b & a) != 0) {
                    skip_instruction(cpu);
                    cpu->cycles++;
                }
                cpu->cycles += 2;
                break;
            }

            case OP_IFE: {
                if (b != a) {
                    skip_instruction(cpu);
                    cpu->cycles++;
                }
                cpu->cycles += 2;
                break;
            }

            case OP_IFN: {
                if (b == a) {
                    skip_instruction(cpu);
                    cpu->cycles++;
                }
                cpu->cycles += 2;
                break;
            }

            case OP_IFG: {
                if (b <= a) {
                    skip_instruction(cpu);
                    cpu->cycles++;
                }
                cpu->cycles += 2;
                break;
            }

            case OP_IFA: {
                if ((int16_t)b <= (int16_t)a) {
                    skip_instruction(cpu);
                    cpu->cycles++;
                }
                cpu->cycles += 2;
                break;
            }

            case OP_IFL: {
                if (b >= a) {
                    skip_instruction(cpu);
                    cpu->cycles++;
                }
                cpu->cycles += 2;
                break;
            }

            case OP_IFU: {
                if ((int16_t)b >= (int16_t)a) {
                    skip_instruction(cpu);
                    cpu->cycles++;
                }
                cpu->cycles += 2;
                break;
            }

            case OP_ADX: {
                const uint32_t rest = b + a + cpu->ex;
                *ptr_b = rest & 0xffff;
                cpu->ex = (rest > 0xFFFF) ? 0x0001 : 0x0000;
                cpu->cycles += 3;
                break;
            }

            case OP_SBX: {
                uint16_t carry_in = (cpu->ex == 0xFFFF) ? 1 : 0;
                uint32_t rest = (uint32_t)b - (uint32_t)a - carry_in;

                *ptr_b = rest & 0xFFFF;
                cpu->ex = (rest > 0xFFFF) ? 0xFFFF : 0x0000;
                cpu->cycles += 3;
                break;
            }

            case OP_STI: {
                *ptr_b = a;
                cpu->reg[I]++;
                cpu->reg[J]++;
                cpu->cycles++;
                break;
            }

            case OP_STD: {
                *ptr_b = a;
                cpu->reg[I]--;
                cpu->reg[J]--;
                cpu->cycles++;
                break;
            }
            default: cpu->cycles++; break;
        }
    }
    return cpu->cycles - initial_cycles;
}


void skip_instruction(DCPU16 *cpu) {
    uint16_t instr = cpu->ram[cpu->pc++];

    uint8_t opcode = instr & 0x1F;
    uint8_t b = (instr >> 5) & 0x1F;
    uint8_t a = (instr >> 10) & 0x3F;

    if ((a >= PTR_REG_NW && a <= 0x17) || a == PTR_NW || a == NW) {
        cpu->pc++;
    }

    if (opcode != 0) {
        if ((b >= PTR_REG_NW && b <= 0x17) || b == PTR_NW || b == NW) {
            cpu->pc++;
        }
    }

    if (opcode >= PTR_REG_NW && opcode <= 0x17) {
        skip_instruction(cpu);
        cpu->cycles += 1;
    }
}


uint16_t* operand_val(DCPU16 *cpu, const uint_fast8_t val, const bool is_a) {
    switch (val) {
        case REG_START ... 0x07:
            return &cpu->reg[val];

        case PTR_REG ... 0x0F:
            return &cpu->ram[cpu->reg[val - PTR_REG]];

        case PTR_REG_NW ... 0x17:
            return  &cpu->ram[cpu->reg[val - PTR_REG_NW] + cpu->ram[cpu->pc++] & 0xFFFF];

        case PUSH_POP:
            if (is_a) {
                return &cpu->ram[cpu->sp++];
            }
            return &cpu->ram[--cpu->sp];

        case PEEK:
            return &cpu->ram[cpu->sp];

        case PICK:
            return &cpu->ram[cpu->sp + cpu->ram[cpu->pc++] & 0xffff];

        case SP:
            return &cpu->sp;

        case PC:
            return &cpu->pc;

        case EX:
            return &cpu->ex;

        case PTR_NW:
            return &cpu->ram[cpu->ram[cpu->pc++]];

        case NW:
            return &cpu->ram[cpu->pc++];

        case LITERAL_START ... 0x3F:
            dummy_literal = (val - 0x21);
            return &dummy_literal;

        default:
            return NULL;

    }
}

static void specop_exec(DCPU16 *cpu, uint16_t *ptr_a, uint16_t a, uint16_t opcode) {
    switch (opcode)
    {
        case SOP_HAL:{
            cpu->halted = true;
            printf("FATAL: CPU Halted at PC = %04X\n", cpu->pc - 1);
            break;
        }
        case SOP_JSR: {
            cpu->ram[--cpu->sp] = cpu->pc;
            cpu->pc = a;
            cpu->cycles += 3;
            break;
        }
        case SOP_INT: {
            interrupt_enqueue(cpu, a);
            cpu->cycles += 4;
            break;
        }
        case SOP_IAG: {
            *ptr_a = cpu->ia;
            cpu->cycles++;
            break;
        }
        case SOP_IAS: {
            cpu->ia  = a;
            cpu->cycles++;
            break;
        }
        case SOP_RFI: {
            cpu->interrupt_enabled = false;
            cpu->reg[A] = cpu->ram[cpu->sp++];
            cpu->pc = cpu->ram[cpu->sp++];
            cpu->cycles += 3;
            break;
        }
        case SOP_IAQ: {
            cpu->interrupt_enabled = a != 0 ? true : false;
            cpu->cycles += 2;
            break;
        }
        case SOP_HWN: {
            *ptr_a =  cpu->num_hardware;
            cpu->cycles += 2;
            break;
        }
        case SOP_HWQ: {
            if (a < cpu->num_hardware) {
                DCPU_Hardware *hw = cpu->bus[a];

                cpu->reg[A] = hw->hardware_id & 0xFFFF;
                cpu->reg[B] = (hw->hardware_id >> 16) & 0xFFFF;

                cpu->reg[C] = hw->hardware_version;

                cpu->reg[X] = hw->manufacturer & 0xFFFF;
                cpu->reg[Y] = (hw->manufacturer >> 16) & 0xFFFF;
            } else {
                cpu->reg[A] = 0;
                cpu->reg[B] = 0;
                cpu->reg[C] = 0;
                cpu->reg[X] = 0;
                cpu->reg[Y] = 0;
            }
            cpu->cycles += 4;
            break;
        }
        case SOP_HWI: {
            if (a < cpu->num_hardware) {
                DCPU_Hardware *hw = cpu->bus[a];

                if (hw->handle_hwi != NULL) {
                    hw->handle_hwi(hw, cpu);
                }
            }
            cpu->cycles += 4;
            break;
        }
        default: cpu->cycles++; break;
    }
}

void interrupt_enqueue(DCPU16 *cpu, uint16_t message) {
    if (cpu->iq_count >= 256) {
        cpu->is_on_fire = true;
        return;
    }

    cpu->interruptq[cpu->iq_head] = message;
    cpu->iq_head = (cpu->iq_head + 1) % 256;
    cpu->iq_count++;
}

void interrupt_dequeue(DCPU16 *cpu) {
    if (!cpu->interrupt_enabled && cpu->iq_count > 0) {

        uint16_t message = cpu->interruptq[cpu->iq_tail];
        cpu->iq_tail = (cpu->iq_tail + 1) % 256;
        cpu->iq_count--;

        if (cpu->ia == 0) {
            return;
        }

        cpu->interrupt_enabled = true;

        cpu->ram[--cpu->sp] = cpu->pc;
        cpu->ram[--cpu->sp] = cpu->reg[A];

        cpu->pc = cpu->ia;
        cpu->reg[A] = message;
    }
}

void dcpu_add_hardware(DCPU16 *cpu, DCPU_Hardware *hardware) {
    if (cpu == NULL || hardware == NULL) {
        return;
    }

    cpu->num_hardware++;

    cpu->bus = realloc(cpu->bus, cpu->num_hardware * sizeof(DCPU_Hardware*));

    cpu->bus[cpu->num_hardware - 1] = hardware;
}

void free_hardware(DCPU16 *cpu) {
    if (cpu->bus != NULL) {
        free(cpu->bus);
        cpu->bus = NULL;
    }
    cpu->num_hardware = 0;
}
