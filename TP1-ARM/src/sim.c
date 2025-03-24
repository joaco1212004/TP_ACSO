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

void adds_shifted_register(int64_t* xd, int64_t* xn, int64_t* xm) {
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


void process_instruction () {
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. 
     * */

    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    // uint32_t opcode = (instruction >> 21) & 0x7FF;
    // printf("Opcode: 0x%X\n", opcode);
    // printf("Instruction: 0x%X\n", instruction);
    // for (int i = 31; i >= 0; i--) {
    //     printf("%d", (instruction >> i) & 1);
    // }
    // printf("\n");

    // uint32_t part1 = instruction >> 24;
    // uint32_t part2 = (instruction >> 21) & 0b1;
    // printf("Part1: ");
    // for (int i = 7; i >= 0; i--) {
    //     printf("%d", (part1 >> i) & 1);
    // }
    // printf(", Part2: ");
    // for (int i = 0; i >= 0; i--) {
    //     printf("%d", (part2 >> i) & 1);
    // }
    // printf("\n");

    printf("%x\n", instruction >> 24);
    printf("%x\n", (instruction >> 21) & 1);
    if (instruction >> 24 == 0b10110001) { // adds (immediate) C6-531
        printf("I'm in adds immediate\n");

        int64_t rd = instruction & 0x1F;
        int64_t rn = (instruction >> 5) & 0x1F;
        int64_t shift = (instruction >> 22) & 0x3;
        int64_t imm12 = (instruction >> 10) & 0xFFF;

        switch (shift) {
            case 0:
                break;
            case 1:
                imm12 <<= 12;
                break;
            default:
                // Reserved value, HALT
                RUN_BIT = 0;
                return;
        }

        adds_shifted_register(&rd, &rn, &imm12);

        // printf("rd: %ld, rn: %ld, imm12: %ld\n", rd, rn, imm12);
    } else if (instruction >> 24 == 0b10101011 && ((instruction >> 21) & 1) == 0) { // adds (shifted register) C6-533
        printf("I'm in adds shifted register\n");

        int64_t rd = instruction & 0x1F;
        int64_t rn = (instruction >> 5) & 0x1F;
        int64_t rm = (instruction >> 16) & 0x1F;
        int64_t shift = (instruction >> 22) & 0x3;
        int64_t imm6 = (instruction >> 10) & 0x3F;

        if (shift == 0b11) {
            // Reserved value, HALT
            RUN_BIT = 0;
            return;
        }

        ShiftType shift_type = DecodeShift(shift);
        int64_t shift_amount = imm6;

        int64_t shifted_rm = CURRENT_STATE.REGS[rm];
        switch (shift_type) {
            case LSL:
                shifted_rm <<= shift_amount;
                break;
            case LSR:
                shifted_rm = (uint64_t)shifted_rm >> shift_amount;
                break;
            case ASR:
                shifted_rm >>= shift_amount;
                break;
            default:
                // Reserved value, HALT
                RUN_BIT = 0;
                return;
        }

        adds_shifted_register(&rd, &rn, &shifted_rm);


    // } else if () { // subs (shifted register) C6-938
        
    }

    CURRENT_STATE.PC += 4;
    NEXT_STATE.PC = CURRENT_STATE.PC;

    if (instruction == 0xd4400000) { // HLT 0
        RUN_BIT = 0;
    }
}