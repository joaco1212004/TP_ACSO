#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"

typedef enum {
    LSL,
    LSR,
    ASR
} ShiftType;

ShiftType DecodeShift(uint32_t shift) {
    switch (shift) {
        case 0:
            return LSL;
        case 1:
            return LSR;
        case 2:
            return ASR;
        default:
            // Reserved value, HALT
            RUN_BIT = 0;
            return LSL; // Default return to avoid compiler warning
    }
}

uint64_t ZeroExtend(uint64_t value, int n) {
    // Mask the value to ensure it fits within n bits
    return value & ((1ULL << n) - 1);
}

uint64_t Zeros(int n) {
    // Return a value with n zero bits
    return 0;
}

void check_flags(int64_t result) {
    if (result < 0) {
        NEXT_STATE.FLAG_N = 1;
    } else {
        NEXT_STATE.FLAG_N = 0;
    }

    if (result == 0) {
        NEXT_STATE.FLAG_Z = 1;
    } else {
        NEXT_STATE.FLAG_Z = 0;
    }
}

void adc(int64_t* xd, int64_t* xn, int64_t* xm) {
    // CURRENT_STATE.REGS[*xd] = CURRENT_STATE.REGS[*xn] + CURRENT_STATE.REGS[*xm];
    NEXT_STATE.REGS[*xd] = CURRENT_STATE.REGS[*xn] + *xm;
    // mem_write_32(NEXT_STATE.REGS[*xd], CURRENT_STATE.REGS[*xn] + *xm);
    // NEXT_STATE.FLAG_N = CURRENT_STATE.FLAG_N;
    // NEXT_STATE.FLAG_Z = CURRENT_STATE.FLAG_Z;
    
    int64_t result = CURRENT_STATE.REGS[*xn] + *xm;
    NEXT_STATE.REGS[*xd] = result;

    // Update flags
    // Negative flag (N): Set if the result is negative
    if (result < 0) {
        NEXT_STATE.FLAG_N = 1;
    } else {
        NEXT_STATE.FLAG_N = 0;
    }

    // Zero flag (Z): Set if the result is zero
    if (result == 0) {
        NEXT_STATE.FLAG_Z = 1;
    } else {
        NEXT_STATE.FLAG_Z = 0;
    }
}

void adds_immediate(int64_t instruction) {
    int64_t rd = instruction & 0x1F;
    int64_t rn = (instruction >> 5) & 0x1F;
    int64_t shift = (instruction >> 22) & 0x3;
    int64_t imm12 = (instruction >> 10) & 0xFFF;

    switch (shift) {
        case 0:
            // ZeroExtend(imm12, 64);
            break;
        case 1:
            imm12 <<= 12;
            break;
        default:
            RUN_BIT = 0;
            return;
    }

    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] + imm12;
    
    int64_t result = CURRENT_STATE.REGS[rn] + imm12;
    NEXT_STATE.REGS[rd] = result;

    check_flags(result);
}

void adds_extended_register(int64_t instruction) {
    int64_t rd = instruction & 0x1F;
    int64_t rn = (instruction >> 5) & 0x1F;
    int64_t rm = (instruction >> 16) & 0x1F;
    int64_t imm3 = (instruction >> 10) & 0x7;
    int64_t option = (instruction >> 13) & 0x7;
    
    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] + rm; // Usamos el mem_write_32 para escribir en el registro ?
    
    int64_t result = CURRENT_STATE.REGS[rn] + rm;
    NEXT_STATE.REGS[rd] = result;

    check_flags(result);
}

void subs_extended_register(int64_t instruction) {
    int64_t d = instruction & 0x1F;
    int64_t n = (instruction >> 5) & 0x1F;
    int64_t m = (instruction >> 16) & 0x1F;
    int shift = (instruction >> 10) & 0x7;
    int64_t option = (instruction >> 13) & 0x7;

    ShiftType extend_type = DecodeShift(option); // Decode the shift type

    if (shift > 4) {
        RUN_BIT = 0;
        return;
    }
    NEXT_STATE.REGS[d] = CURRENT_STATE.REGS[n] - m;
    
    int64_t result = CURRENT_STATE.REGS[n] - m;
    NEXT_STATE.REGS[d] = result;

    check_flags(result);
}

void subs_immediate(int64_t instruction) {
    int64_t rd = instruction & 0x1F;
    int64_t rn = (instruction >> 5) & 0x1F;
    int64_t shift = (instruction >> 22) & 0x3;
    int64_t imm12 = (instruction >> 10) & 0xFFF;

    switch (shift) {
        case 0:
            // ZeroExtend(imm12, 64);
            break;
        case 1:
            imm12 <<= 12;
            break;
        default:
            RUN_BIT = 0;
            return;
    }

    int64_t result = CURRENT_STATE.REGS[rn] - imm12;
    NEXT_STATE.REGS[rd] = result;

    check_flags(result);
}

void add_immediate (int64_t instruction){
    int64_t rd = instruction & 0x1F; 
    int64_t rn = (instruction >> 5) & 0x1F;
    int64_t imm12 = (instruction >> 10) & 0xFFF;
    int64_t shift = (instruction >> 22) & 0x3;

    switch (shift) {
        case 0:
            // ZeroExtend(imm12, 64);
            break;
        case 1:
            imm12 <<= 12;
            break;
        default:
            RUN_BIT = 0;
            return;
    }

    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] + imm12;
}

void add_extended (int64_t instruction){
    int64_t rd = instruction & 0x1F; 
    int64_t rn = (instruction >> 5) & 0x1F;
    int64_t rm = (instruction >> 16) & 0x1F;

    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];
}

void adds_shifted_register(int64_t instruction) {
    int64_t d = instruction & 0x1F;
    int64_t n = (instruction >> 5) & 0x1F;
    int64_t imm6 = (instruction >> 10) & 0x3F;
    int64_t m = (instruction >> 16) & 0x1F;
    int shift = (instruction >> 22) & 0x3;

    if (shift == 0b11) {
        RUN_BIT = 0;
        return;
    }

    ShiftType shift_type = DecodeShift(shift); // Decode the shift type
    int64_t shift_amount = imm6;
    //int64_t shift_amount = m;

    switch (shift_type) {
        case LSL:
            NEXT_STATE.REGS[d] = CURRENT_STATE.REGS[n] + (CURRENT_STATE.REGS[m] << shift_amount);
            break;
        case LSR:
            NEXT_STATE.REGS[d] = CURRENT_STATE.REGS[n] + (CURRENT_STATE.REGS[m] >> shift_amount);
            break;
        case ASR:
            NEXT_STATE.REGS[d] = CURRENT_STATE.REGS[n] + (CURRENT_STATE.REGS[m] >> shift_amount);
            break;
    }

    int64_t result = 0;
    switch (shift_type) {
        case LSL:
            result = CURRENT_STATE.REGS[n] + (CURRENT_STATE.REGS[m] << shift_amount);
            break;
        case LSR:
            result = CURRENT_STATE.REGS[n] + (CURRENT_STATE.REGS[m] >> shift_amount);
            break;
        case ASR:
            result = CURRENT_STATE.REGS[n] + (CURRENT_STATE.REGS[m] >> shift_amount);
            break;
    }
    NEXT_STATE.REGS[d] = result;

    check_flags(result);
}

void mul (int64_t instruction){
    int64_t rd = instruction & 0x1F; 
    int64_t rn = (instruction >> 5) & 0x1F;
    int64_t rm = (instruction >> 16) & 0x1F;

    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] * CURRENT_STATE.REGS[rm];
}

void ands(int64_t instruction) {
    int64_t d = instruction & 0x1F;
    int64_t n = (instruction >> 5) & 0x1F;
    int64_t m = (instruction >> 16) & 0x1F;

    NEXT_STATE.REGS[d] = CURRENT_STATE.REGS[n] & CURRENT_STATE.REGS[m];
    
    int64_t result = CURRENT_STATE.REGS[n] & CURRENT_STATE.REGS[m];
    NEXT_STATE.REGS[d] = result;

    check_flags(result);
}

void cmp_extended_register(int64_t instruction) {
    int64_t n = (instruction >> 5) & 0x1F;
    int64_t m = (instruction >> 16) & 0x1F;

    int64_t result = CURRENT_STATE.REGS[n] - m;

    // Update flags
    check_flags(result);
}

// void cmp_immediate(int64_t instruction) {
//     int64_t n = (instruction >> 5) & 0x1F;
//     int64_t imm12 = (instruction >> 10) & 0xFFF;

//     int64_t result = CURRENT_STATE.REGS[n] - imm12;

//     // Update flags
//     check_flags(result);
// }

void cmp_extended(int64_t instruction) {
    int64_t rd = instruction & 0x1F; // registro 0, no alterar su valor
    int64_t rn = (instruction >> 5) & 0x1F;
    int64_t rm = (instruction >> 16) & 0x1F;

    if ((CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm]) < 0) {
        NEXT_STATE.FLAG_N = 1;
    } else if ((CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm]) == 0) {
        NEXT_STATE.FLAG_Z = 1;
    } else {
        NEXT_STATE.FLAG_N = 0;
        NEXT_STATE.FLAG_Z = 0;
    }
}

void cmp_immediate (int64_t instruction) {
    int64_t rd = instruction & 0x1F; // registro 0, no alterar su valor
    int64_t rn = (instruction >> 5) & 0x1F;
    int64_t imm12 = (instruction >> 10) & 0xFFF;

    if (CURRENT_STATE.REGS[rn] - imm12 < 0) {
        NEXT_STATE.FLAG_N = 1;
    } else if (CURRENT_STATE.REGS[rn] - imm12 == 0) {
        NEXT_STATE.FLAG_Z = 1;
    } else {
        NEXT_STATE.FLAG_N = 0;
        NEXT_STATE.FLAG_Z = 0;
    }
}

void eor_shift_reg(int64_t* xd, int64_t* xn, int64_t* xm) {
    // shift is always 0
    int64_t result = CURRENT_STATE.REGS[*xn] ^ CURRENT_STATE.REGS[*xm];

    NEXT_STATE.REGS[*xd] = result;

    check_flags(result);
}

void orr_shift_reg(int64_t* xd, int64_t* xn, int64_t* xm) {
    // shift is always 0
    int64_t result = CURRENT_STATE.REGS[*xn] | CURRENT_STATE.REGS[*xm];

    NEXT_STATE.REGS[*xd] = result;

    check_flags(result);
}

void b_target(int32_t imm26) {
    // Calculate the target address relative to the current PC
    int64_t offset = (int64_t)(imm26 << 2);
    if (offset & (1 << 27)) {
        offset |= ~((1 << 28) - 1);
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + offset;
}

void br(int64_t* xn) {
    NEXT_STATE.PC = CURRENT_STATE.REGS[*xn];
}

void b_conditional(int32_t imm19, int cond) {
    int64_t offset = (int64_t)(imm19 << 2);
    if (offset & (1 << 20)) {
        offset |= ~((1 << 21) - 1);
    }

    switch (cond) {
        case 0: // BEQ
            if (NEXT_STATE.FLAG_Z == 1) {
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                return;
            }
            break;
        case 1: // BNE
            if (NEXT_STATE.FLAG_Z == 0) {
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                return;
            }
            break;
        case 12: // BGE
            if (NEXT_STATE.FLAG_N == 0) {
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                return;
            }
            break;
        case 13: // BLT
            if (NEXT_STATE.FLAG_N == 1) {
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                return;
            }
            break;
        case 14: // BGT
            if (NEXT_STATE.FLAG_Z == 0 && NEXT_STATE.FLAG_N == 0) {
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                return;
            }
            break;
        case 15: // BLE
            if (NEXT_STATE.FLAG_Z == 1 || NEXT_STATE.FLAG_N == 1) {
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                return;
            }
            break;
        default:
            RUN_BIT = 0; // Invalid condition
            return;
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + 4; // No branch taken
}
