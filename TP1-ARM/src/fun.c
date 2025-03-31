#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"

typedef enum { // TODO: hacer archivos nuevos de test para probar las funciones que no se probaron en loa archivos dados y probar casos bordes para sacornos un 10
    LSL,
    LSR,
    ASR
} ShiftType;

ShiftType DecodeShift(uint32_t shift) { // TODO: Ordenar todas las funciones
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

// void adc(int64_t* xd, int64_t* xn, int64_t* xm) { // TODO: remove adc si no lo necesitamos
//     // CURRENT_STATE.REGS[*xd] = CURRENT_STATE.REGS[*xn] + CURRENT_STATE.REGS[*xm];
//     NEXT_STATE.REGS[*xd] = CURRENT_STATE.REGS[*xn] + *xm;
//     // mem_write_32(NEXT_STATE.REGS[*xd], CURRENT_STATE.REGS[*xn] + *xm);
//     // NEXT_STATE.FLAG_N = CURRENT_STATE.FLAG_N;
//     // NEXT_STATE.FLAG_Z = CURRENT_STATE.FLAG_Z;
    
//     int64_t result = CURRENT_STATE.REGS[*xn] + *xm;
//     NEXT_STATE.REGS[*xd] = result;

//     // Update flags
//     // Negative flag (N): Set if the result is negative
//     if (result < 0) {
//         NEXT_STATE.FLAG_N = 1;
//     } else {
//         NEXT_STATE.FLAG_N = 0;
//     }

//     // Zero flag (Z): Set if the result is zero
//     if (result == 0) {
//         NEXT_STATE.FLAG_Z = 1;
//     } else {
//         NEXT_STATE.FLAG_Z = 0;
//     }
// }

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

    int64_t result = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];
    NEXT_STATE.REGS[rd] = result;

    check_flags(result);
}

void subs_extended_register(int64_t instruction) {
    int64_t d = instruction & 0x1F;
    int64_t n = (instruction >> 5) & 0x1F;
    int64_t m = (instruction >> 16) & 0x1F;

    int64_t result = CURRENT_STATE.REGS[n] - CURRENT_STATE.REGS[m];
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

    int64_t result = CURRENT_STATE.REGS[rn] * CURRENT_STATE.REGS[rm];
    NEXT_STATE.REGS[rd] = result;
    check_flags(result);
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void ands(int64_t instruction) {
    int64_t d = instruction & 0x1F;
    int64_t n = (instruction >> 5) & 0x1F;
    int64_t m = (instruction >> 16) & 0x1F;

    NEXT_STATE.REGS[d] = CURRENT_STATE.REGS[n] & CURRENT_STATE.REGS[m];
    
    int64_t result = CURRENT_STATE.REGS[n] & CURRENT_STATE.REGS[m];
    NEXT_STATE.REGS[d] = result;

    check_flags(result);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void cmp_extended_register(int64_t instruction) {
    int64_t n = (instruction >> 5) & 0x1F;
    int64_t m = (instruction >> 16) & 0x1F;

    int64_t result = CURRENT_STATE.REGS[n] - CURRENT_STATE.REGS[m];

    // Update flags
    check_flags(result);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
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

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
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

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void eor_shifted_register(int64_t instruction) {
    int64_t d = instruction & 0x1F;
    int64_t n = (instruction >> 5) & 0x1F;
    int64_t m = (instruction >> 16) & 0x1F;

    int64_t result = CURRENT_STATE.REGS[n] ^ CURRENT_STATE.REGS[m];

    NEXT_STATE.REGS[d] = result;

    check_flags(result);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void eor_immediate(int64_t instruction) {

    int64_t d = instruction & 0x1F;
    int64_t n = (instruction >> 5) & 0x1F;
    int64_t imm12 = (instruction >> 10) & 0xFFF;

    int64_t result = CURRENT_STATE.REGS[n] ^ imm12;

    NEXT_STATE.REGS[d] = result;

    check_flags(result);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void orr_shift_reg(int64_t instruction) {
    int64_t d = instruction & 0x1F;
    int64_t n = (instruction >> 5) & 0x1F;
    int64_t m = (instruction >> 16) & 0x1F;

    int64_t result = CURRENT_STATE.REGS[n] | CURRENT_STATE.REGS[m];

    NEXT_STATE.REGS[d] = result;

    check_flags(result);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void b_target(int64_t instruction) { // TODO: hacer que todas las funciones reciban únicamente instruction para que el sim.c esté más limpio.
    int32_t imm26 = instruction & 0x3FFFFFF;
    // Calculate the target address relative to the current PC
    int64_t offset = (int64_t)(imm26 << 2);
    if (offset & (1 << 27)) {
        offset |= ~((1 << 28) - 1);
    }

    NEXT_STATE.PC = CURRENT_STATE.PC + offset;
}

void br(int64_t instruction) {
    int64_t xn = instruction & 0x1F;

    NEXT_STATE.PC = CURRENT_STATE.REGS[xn];
}

void b_conditional(int64_t instruction) {
    int32_t imm19 = (instruction >> 5) & 0x7FFFF;
    //int cond = (instruction >> 12) & 0xF;
    int cond = instruction & 0xF;

    int64_t offset = ((int64_t)(imm19 << 2) << 43) >> 43;

    // if (offset & (1 << 20)) {
    //     offset |= ~((1 << 21) - 1);
    // }

    if (cond == 0x0) { // BEQ
        if (NEXT_STATE.FLAG_Z == 1) {
            NEXT_STATE.PC += offset;
        }
        else { NEXT_STATE.PC += 4; }
    }
    else if (cond == 0x1) { // BNE
        if (NEXT_STATE.FLAG_Z == 0) {
            NEXT_STATE.PC += offset;
        }
        else { NEXT_STATE.PC += 4; }
    }
    else if (cond == 0xC) { // BGT
        if (NEXT_STATE.FLAG_Z == 0 && NEXT_STATE.FLAG_N == 0) {
            NEXT_STATE.PC += offset;
        }
        else { NEXT_STATE.PC += 4; }
    }
    else if (cond == 0xB) { // BLT
        if (NEXT_STATE.FLAG_N != 0) {
            NEXT_STATE.PC += offset;
        }
        else { NEXT_STATE.PC += 4; }
    }
    else if (cond == 0xA) { // BGE
        if (NEXT_STATE.FLAG_N == 0) {
            NEXT_STATE.PC += offset;
        }
        else { NEXT_STATE.PC += 4; }
    }
    else if (cond == 0xD) { // BLE
        if (!(NEXT_STATE.FLAG_Z == 0 && NEXT_STATE.FLAG_N == 0)) {
            NEXT_STATE.PC += offset;
        }
        else { NEXT_STATE.PC += 4; }
    }
    else {
        return;
    }
}

void lsl(int64_t instruction) {
    int64_t d = instruction & 0x1F;
    int64_t n = (instruction >> 5) & 0x1F;
    int64_t immr = (instruction >> 16) & 0x3F; // este valor debería de ser 3F no 1F pero con 1F funciona

    // Perform the LSL operation with the immediate value
    int64_t result = CURRENT_STATE.REGS[n] << (64 - immr);

    // Store the result in the destination register
    NEXT_STATE.REGS[d] = result;

    // Update flags
    check_flags(result);
}

void lsr(int64_t instruction) {
    int64_t d = instruction & 0x1F;
    int64_t n = (instruction >> 5) & 0x1F;
    int64_t immr = (instruction >> 16) & 0x3F; // este valor debería de ser 3F no 1F pero con 1F funciona

    // Perform the LSR operation with the immediate value
    int64_t result = CURRENT_STATE.REGS[n] >> immr;

    // Store the result in the destination register
    NEXT_STATE.REGS[d] = result;

    // Update flags
    check_flags(result);

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void stur(int64_t instruction) {
    uint64_t rt = instruction & 0x1F; // Destination register
    uint64_t rn = (instruction >> 5) & 0x1F; // Base register
    int64_t offset = (((instruction >> 14) & 0x1FF) << 55) >> 55; // Immediate value shifteado a offset
    uint64_t effective_address = CURRENT_STATE.REGS[rn] + offset;

    // Store the value from the register into memory
    mem_write_32(effective_address, CURRENT_STATE.REGS[rt]);

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void sturb(int64_t instruction) {
    uint64_t rt = instruction & 0x1F; // Destination register
    uint64_t rn = (instruction >> 5) & 0x1F; // Base register
    int64_t offset = (((instruction >> 14) & 0x1FF) << 55) >> 55; // Immediate value shifteado a offset
    uint64_t effective_address = CURRENT_STATE.REGS[rn] + offset;
    uint8_t data = (int8_t)NEXT_STATE.REGS[rt];
    uint32_t dataExtended = (int32_t)((data << 24) >> 24);
    uint32_t writeDATA = (mem_read_32(effective_address) & 0xFFFFFF00) + dataExtended; // Reservo los ocho ultimos bits de memoria para guardar
    mem_write_32(effective_address, writeDATA);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void sturh (int64_t instruction){
    uint64_t rt = instruction & 0x1F; // Destination register
    uint64_t rn = (instruction >> 5) & 0x1F; // Base register
    int64_t offset = (((instruction >> 14) & 0x1FF) << 55) >> 55; // Immediate value shifteado a offset
    uint64_t effective_address = CURRENT_STATE.REGS[rn] + offset;
    uint16_t data = (int8_t)NEXT_STATE.REGS[rt];
    uint32_t dataExtended = (int32_t)((data << 16) >> 16);
    uint32_t writeData = (mem_read_32(effective_address) & 0xffff0000) | dataExtended;
    mem_write_32(effective_address, writeData);

    NEXT_STATE.PC += 4;
}

void movz(int64_t instruction) {
    int64_t d = instruction & 0x1F;
    int64_t imm16 = (instruction >> 5) & 0xFFFF;
    int64_t shift = (instruction >> 21) & 0x3;

    // Perform the MOVZ operation with the immediate value
    int64_t result = imm16 << (shift * 16);

    // Store the result in the destination register
    NEXT_STATE.REGS[d] = result;

    // Update flags
    check_flags(result);

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void cbz(int64_t instruction) {
    int64_t rt = instruction & 0x1F; // Register to check
    int32_t imm19 = (instruction >> 5) & 0x7FFFF; // Immediate value (19 bits)

    // Sign-extend the immediate value to 64 bits
    int64_t offset = (imm19 & 0x40000) ? (imm19 | 0xFFFFFFFFFFF80000) : imm19;

    // Calculate the branch target address
    int64_t target_address = CURRENT_STATE.PC + (offset << 2);

    // Check if the register value is zero
    if (CURRENT_STATE.REGS[rt] == 0) {
        NEXT_STATE.PC = target_address;
    } else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4; // Move to the next instruction
    }
}

void cbnz(int64_t instruction) {
    int64_t rt = instruction & 0x1F; // Register to check
    int32_t imm19 = (instruction >> 5) & 0x7FFFF; // Immediate value (19 bits)

    // Sign-extend the immediate value to 64 bits
    int64_t offset = (imm19 & 0x40000) ? (imm19 | 0xFFFFFFFFFFF80000) : imm19;

    // Calculate the branch target address
    int64_t target_address = CURRENT_STATE.PC + (offset << 2);

    // Check if the register value is not zero
    if (CURRENT_STATE.REGS[rt] != 0) {
        NEXT_STATE.PC = target_address;
    } else {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4; // Move to the next instruction
    }
}

// void ldur(int64_t instruction) {
//     int64_t rt = instruction & 0x1F; // Destination register
//     int64_t rn = (instruction >> 5) & 0x1F; // Base register
//     int64_t imm9 = (instruction >> 12) & 0x1FF; // Immediate value

//     // Sign-extend the immediate value to 64 bits
//     int64_t offset = (imm9 & 0x100) ? (imm9 | 0xFFFFFFFFFFFFFF00) : imm9;

//     // Calculate the effective address
//     int64_t effective_address = CURRENT_STATE.REGS[rn] + offset;

//     // Simulate a 64-bit memory read by combining two 32-bit reads
//     uint64_t lower = (uint64_t)mem_read_32(effective_address);          // Read lower 32 bits
//     uint64_t upper = (uint64_t)mem_read_32(effective_address + 4);      // Read upper 32 bits
//     uint64_t value = (upper << 32) | lower;                             // Combine into 64-bit value

//     // Store the 64-bit value in the destination register
//     NEXT_STATE.REGS[rt] = value;

//     // Update the PC
//     NEXT_STATE.PC = CURRENT_STATE.PC + 4;
// }

// void ldurb(int64_t instruction) {
//     int64_t rt = instruction & 0x1F; // Destination register
//     int64_t rn = (instruction >> 5) & 0x1F; // Base register
//     int64_t imm9 = (instruction >> 12) & 0x1FF; // Immediate value

//     // Sign-extend the immediate value to 64 bits
//     int64_t offset = (imm9 & 0x100) ? (imm9 | 0xFFFFFFFFFFFFFF00) : imm9;

//     // Calculate the effective address
//     int64_t effective_address = CURRENT_STATE.REGS[rn] + offset;

//     // Simulate an 8-bit memory read
//     uint8_t value = (uint8_t)mem_read_32(effective_address);

//     // Store the 8-bit value in the destination register (zero-extended)
//     NEXT_STATE.REGS[rt] = (uint64_t)value;

//     // Update the PC
//     NEXT_STATE.PC = CURRENT_STATE.PC + 4;
// }


void ldur(int64_t instruction) {
    uint64_t rt = instruction & 0x1F; // Destination register
    uint64_t rn = (instruction >> 5) & 0x1F; // Base register
    int64_t offset = (((instruction >> 14) & 0x1FF) << 55) >> 55; // Immediate value shifteado a offset
    uint32_t lower4Bytes = mem_read_32(NEXT_STATE.REGS[rn] + offset);
    uint32_t upper4Bytes = mem_read_32(NEXT_STATE.REGS[rn] + offset + 4);
    NEXT_STATE.REGS[rt] = ((uint64_t)upper4Bytes << 32) | lower4Bytes;

    // // Sign-extend the immediate value to 64 bits
    // int64_t offset = (imm9 & 0x100) ? (imm9 | 0xFFFFFFFFFFFFFE00) : imm9;

    // // Calculate the effective address
    // int64_t effective_address;
    // if (rn == 31) { // If Rn == 31, use REGS[31] as the stack pointer
    //     effective_address = CURRENT_STATE.REGS[31] + offset;
    // } else { // Otherwise, use the base register
    //     effective_address = CURRENT_STATE.REGS[rn] + offset;
    // }

    // // Simulate a 64-bit memory read by combining two 32-bit reads
    // uint64_t lower = (uint64_t)mem_read_32(effective_address);          // Read lower 32 bits
    // uint64_t upper = (uint64_t)mem_read_32(effective_address + 4);      // Read upper 32 bits
    // uint64_t value = (upper << 32) | lower;                             // Combine into 64-bit value

    // Store the 64-bit value in the destination register
    // NEXT_STATE.REGS[rt] = value;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void ldurb(int64_t instruction) {
    uint64_t rt = instruction & 0x1F; // Destination register
    uint64_t rn = (instruction >> 5) & 0x1F; // Base register
    int64_t offset = (((instruction >> 14) & 0x1FF) << 55) >> 55; // Immediate value shifteado a offset
    int64_t data = (int64_t)(mem_read_32(CURRENT_STATE.REGS[rn] + offset) & 0xFF);
    CURRENT_STATE.REGS[rt] = data;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}

void ldurh(int64_t instruction) {
    uint64_t rt = instruction & 0x1F; // Destination register
    uint64_t rn = (instruction >> 5) & 0x1F; // Base register
    int64_t offset = (((instruction >> 14) & 0x1FF) << 55) >> 55; // Immediate value shifteado a offset
    int64_t data = (int64_t)(mem_read_32(CURRENT_STATE.REGS[rn] + offset) & 0xFFFF);
    CURRENT_STATE.REGS[rt] = data;

    // Update the PC
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}





































// void stur(int64_t instruction) {
//     int64_t rt = instruction & 0x1F; // Destination register
//     int64_t rn = (instruction >> 5) & 0x1F; // Base register
//     int64_t imm9 = (instruction >> 12) & 0x1FF; // Immediate value

//     // Sign-extend the immediate value to 64 bits
//     int64_t offset = (imm9 & 0x100) ? (imm9 | 0xFFFFFFFFFFFFFF00) : imm9;

//     // Calculate the effective address
//     int64_t effective_address = CURRENT_STATE.REGS[rn] + offset;

//     // Store the 64-bit value from the register into memory
//     mem_write_64(effective_address, CURRENT_STATE.REGS[rt]);

//     // Update the PC
//     NEXT_STATE.PC = CURRENT_STATE.PC + 4;
// }

// void sturb(int64_t instruction) {
//     int64_t rt = instruction & 0x1F; // Destination register
//     int64_t rn = (instruction >> 5) & 0x1F; // Base register
//     int64_t imm9 = (instruction >> 12) & 0x1FF; // Immediate value

//     // Sign-extend the immediate value to 64 bits
//     int64_t offset = (imm9 & 0x100) ? (imm9 | 0xFFFFFFFFFFFFFF00) : imm9;

//     // Calculate the effective address
//     int64_t effective_address = CURRENT_STATE.REGS[rn] + offset;

//     // Store the least significant byte (8 bits) of the register into memory
//     mem_write_8(effective_address, CURRENT_STATE.REGS[rt] & 0xFF);

//     // Update the PC
//     NEXT_STATE.PC = CURRENT_STATE.PC + 4;
// }

// void ldur(int64_t instruction) {
//     int64_t rt = instruction & 0x1F; // Destination register
//     int64_t rn = (instruction >> 5) & 0x1F; // Base register
//     int64_t imm9 = (instruction >> 12) & 0x1FF; // Immediate value

//     // Sign-extend the immediate value to 64 bits
//     int64_t offset = (imm9 & 0x100) ? (imm9 | 0xFFFFFFFFFFFFFF00) : imm9;

//     // Calculate the effective address
//     int64_t effective_address = CURRENT_STATE.REGS[rn] + offset;

//     // Load a 64-bit value from memory into the destination register
//     NEXT_STATE.REGS[rt] = mem_read_64(effective_address);

//     // Update the PC
//     NEXT_STATE.PC = CURRENT_STATE.PC + 4;
// }

// void ldurb(int64_t instruction) {
//     int64_t rt = instruction & 0x1F; // Destination register
//     int64_t rn = (instruction >> 5) & 0x1F; // Base register
//     int64_t imm9 = (instruction >> 12) & 0x1FF; // Immediate value

//     // Sign-extend the immediate value to 64 bits
//     int64_t offset = (imm9 & 0x100) ? (imm9 | 0xFFFFFFFFFFFFFF00) : imm9;

//     // Calculate the effective address
//     int64_t effective_address = CURRENT_STATE.REGS[rn] + offset;

//     // Load an 8-bit value from memory into the destination register (zero-extended)
//     NEXT_STATE.REGS[rt] = mem_read_8(effective_address) & 0xFF;

//     // Update the PC
//     NEXT_STATE.PC = CURRENT_STATE.PC + 4;
// }



// // // TODO: testear que todos los archivos de test corren bien comparando con el excel