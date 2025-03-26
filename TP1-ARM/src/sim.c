#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "fun.c"

void process_instruction () {
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. 
     * */

    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);

    // printf("%x\n", instruction >> 24);
    // printf("%x\n", (instruction >> 21) & 1);
    // printf("%x\n", (instruction));
    for (int i = 31; i >= 0; i--) {
        printf("%d", (instruction >> i) & 1);
        if (i % 4 == 0) printf(" ");
    }
    printf("\n");

    if (instruction >> 24 == 0b10110001) { // adds (immediate) C6-531
        printf("I'm in adds immediate\n");
        adds_immediate(instruction);

    } else if (instruction >> 21 == 0b10101011001) { // adds (extended register) C6-529
        printf("I'm in adds extended register\n");
        adds_extended_register(instruction);

    } else if (instruction >> 21 == 0b11101011001) {
        if ((instruction & 0x1F) == 0b11111) { // cmp (extended register) C6-589
            printf("I'm in cmp extended register\n");   
            cmp_extended_register(instruction);

        } else {                                 // subs (extended register) C6-934
            printf("I'm in subs extended register\n");
            subs_extended_register(instruction);

        }
    } else if (((instruction >> 21) == 0b11101011000) && ((instruction & 0b11111) == 0b11111)){ // CMP (immediate) C6-589 y 591
        printf("I'm in CMP immediate\n");

        cmp_extended(instruction);

    } else if (instruction >> 24 == 0b11110001) {
        if ((instruction & 0x1F) == 0b11111) { // cmp (immediate) C6-591
            printf("I'm in cmp immediate\n");
            cmp_immediate(instruction);

        } else {                              // subs (immediate) C6-936
            printf("I'm in subs immediate\n");
            subs_immediate(instruction);
        }
    } else if (instruction >> 24 == 0b10101011 && ((instruction >> 21) & 1) == 0) { // adds (shifted register) C6-533
        printf("I'm in adds shifted register\n");
        adds_shifted_register(instruction);

    } if (instruction >> 21 == 0b11101010000) { // ands (shifted register) C6-542
        printf("I'm in ands\n");
        ands(instruction);
        
    // } else if (instruction >> 21 == 0b11101011001 && (instruction & 0x1F) == 0x1F) { // cmp (extended register) C6-589
    //     printf("I'm in cmp extended register\n");

    //     int64_t n = (instruction >> 5) & 0x1F;
    //     int64_t m = (instruction >> 16) & 0x1F;

    //     cmp_extended_register(&n, &m);
    // } else if (instruction >> 24 == 0b11110001) { // cmp (immediate) C6-591
    //     if ((instruction & 0x1F) == 0b11111) {
    //         printf("I'm in cmp extended register\n");

    //         int64_t n = (instruction >> 5) & 0x1F;
    //         int64_t m = (instruction >> 16) & 0x1F;

    //         cmp_extended_register(&n, &m);
    //     } else {
    //         printf("I'm in cmp immediate\n");

    //         int64_t n = (instruction >> 5) & 0x1F;
    //         int64_t imm12 = (instruction >> 10) & 0xFFF;

    //         cmp_immediate(&n, &imm12);
    //     }
    } else if (instruction >> 22 == 0b1101001000) { // eor (immediate)
        printf("I'm in eor immediate\n");

        int64_t d = instruction & 0x1F;
        int64_t n = (instruction >> 5) & 0x1F;
        int64_t imm12 = (instruction >> 10) & 0xFFF;

        // Perform the EOR operation with the immediate value
        int64_t result = CURRENT_STATE.REGS[n] ^ imm12;

        // Store the result in the destination register
        NEXT_STATE.REGS[d] = result;

        // Update flags
        check_flags(result);
    } else if (instruction >> 21 == 0b11101010010) { // orr (shifted register)
        printf("I'm in orr\n");

        int64_t d = instruction & 0x1F;
        int64_t n = (instruction >> 5) & 0x1F;
        int64_t m = (instruction >> 16) & 0x1F;

        orr_shift_reg(&d, &n, &m);

    } else if (instruction >> 26 == 0b000101) { // b C6-550
        printf("I'm in b\n");

        int32_t imm26 = instruction & 0x3FFFFFF;

        b_target(imm26);
    } else if (instruction >> 10 == 0b1101011000011111000000 && (instruction & 0xF) == 0b00000) { // br C6-562
        printf("I'm in br\n");

        int64_t xn = instruction & 0x1F;

        br(&xn);
    } else if (instruction >> 24 == 0b01010100 && ((instruction >> 4) & 1) == 0) { // b.cond C6-549
        printf("I'm in b conditional\n");

        int32_t imm19 = instruction & 0x7FFFF;
        int cond = (instruction >> 12) & 0xF;

        b_conditional(imm19, cond);
    } else if ((instruction >> 24) == 0b10010001){ // ADD immediate c6-525
        printf("I'm in ADD immediate\n");

        add_immediate(instruction);
    } else if ((instruction >> 21) == 10101011001){ // ADD extended c6-529
        printf("I'm in ADD extended\n");

        add_extended(instruction);
    } else if (((instruction >> 21) == 0b10011011000) && (((instruction >> 10) & 0x1F) == 0b11111)){
        printf("I'm in mul\n");

        mul(instruction);
    }

    CURRENT_STATE.PC += 4;
    NEXT_STATE.PC = CURRENT_STATE.PC;

    if (instruction == 0xd4400000) { // HLT 0
        printf("HLT\n");
        RUN_BIT = 0;
    }
}
