#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "fun.c"

void process_instruction () { // TODO: modificar el makefile para que compile el fun.c (revisar si no nos conviene hacer un archivo más como los ejemplos)
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. 
     * */

    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);

    for (int i = 31; i >= 0; i--) {
        printf("%d", (instruction >> i) & 1);
        if (i % 4 == 0) printf(" ");
    }
    printf("\n");

    if (instruction >> 24 == 0b10110001) { // adds (immediate) C6-531
        printf("I'm in adds immediate\n");
        adds_immediate(instruction);

    } else if (instruction >> 21 == 0b10101011000) { // adds (extended register) C6-529 // TODO: revisar el bit 21 de la instrucción, en el manual es un 1 pero el tp dice que es un 0
        printf("I'm in adds extended register\n");

        adds_extended_register(instruction);
    } else if (instruction >> 21 == 0b11101011000) {
        if ((instruction & 0x1F) == 0b11111) { // cmp (extended register) C6-589
            printf("I'm in cmp extended register\n");   
            cmp_extended_register(instruction);

        } else {                                 // subs (extended register) C6-934 
            printf("I'm in subs extended register\n");
            subs_extended_register(instruction); // 1110 1011 0000 0000 0000 0000 0000 0010

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
    // } else if (instruction >> 24 == 0b10101011 && ((instruction >> 21) & 1) == 0) { // adds (shifted register) C6-533
    //     printf("I'm in adds shifted register\n");
    //     adds_shifted_register(instruction);

    } if (instruction >> 21 == 0b11101010000) { // ands (shifted register) C6-542
        printf("I'm in ands\n");
        ands(instruction);
        
    } else if (instruction >> 22 == 0b1101001000) { // eor (immediate) C6-619
        printf("I'm in eor immediate\n");
        eor_immediate(instruction);

    } else if (instruction >> 21 == 0b11001010000) { // eor (shifted register) C6-620
        printf("I'm in eor shifted register\n");
        eor_shifted_register(instruction);

    } else if (instruction >> 21 == 0b11101010010) { // orr (shifted register)
        printf("I'm in orr\n");
        orr_shift_reg(instruction);

    } else if (instruction >> 26 == 0b000101) { // b C6-550
        printf("I'm in b\n");

        b_target(instruction);
    } else if (instruction >> 10 == 0b1101011000011111000000 && (instruction & 0xF) == 0b00000) { // br C6-562
        printf("I'm in br\n");

        br(instruction);
    } else if (instruction >> 24 == 0b01010100 && ((instruction >> 4) & 1) == 0) { // b.cond C6-549
        printf("I'm in b conditional\n");

        b_conditional(instruction);
    } else if ((instruction >> 24) == 0b10010001){ // ADD immediate c6-525
        printf("I'm in ADD immediate\n");

        add_immediate(instruction);
    } else if ((instruction >> 21) == 10101011001){ // ADD extended c6-529
        printf("I'm in ADD extended\n");

        add_extended(instruction);
    } else if (((instruction >> 21) == 0b10011011000) && (((instruction >> 10) & 0x1F) == 0b11111)){ // mul C6-778
        printf("I'm in mul\n");

        mul(instruction);
    } else if (instruction >> 21 == 0b11010010100) { // movz C6-770
        printf("I'm in movz\n");

        movz(instruction);
    } else if (instruction >> 21 == 0b11111000000 && ((instruction >> 10 & 0b11) == 0)) { // stur C6-917
        printf("I'm in stur\n");
        
        stur(instruction);
    } else if (instruction >> 21 == 0b00111000000 && ((instruction >> 10 & 0b11) == 0)) { // sturb C6-918
        printf("I'm in strub\n");

        sturb(instruction);
    } else if (instruction >> 21 == 0b01111000000 && ((instruction >> 10 & 0b11) == 0)) { // sturh C6-919
        printf("I'm in sturh\n");

        sturh(instruction);
    } else if (instruction >> 21 == 0b11111000010 && ((instruction >> 10 & 0b11) == 0)) { // ldur C6-739
        printf("I'm in ldur\n");

        ldur(instruction);
    } else if (instruction >> 21 == 0b00111000010 && ((instruction >> 10 & 0b11) == 0)) { // ldurb C6-741
        printf("I'm in ldurb\n");

        ldurb(instruction);
    } else if (instruction >> 21 == 0b01111000010 && ((instruction >> 10 & 0b11) == 0)){ // ldurh C6-742
        printf("I'm in ldurh\n");

        ldurh(instruction);
    } else if (((instruction >> 22) == 0b1101001101) && (((instruction >> 10) & 0x3F) != 0b111111)) { // lsl C6-754
        printf("I'm in lsl GUS\n");
        
        lsl(instruction);
    } else if (instruction >> 23 == 0b110100110) { // lsl C6-754
        printf("I'm in lsl Nallib\n");

        lsl(instruction);
    } else if (((instruction >> 22) == 0b1101001101) && (((instruction >> 10) & 0x3F) == 0b111111)) { // lsr C6-757
        printf("I'm in lsr\n");

        lsr(instruction);
    }

    NEXT_STATE.PC = CURRENT_STATE.PC;
    NEXT_STATE.PC += 4;

    if (instruction == 0xd4400000) { // HLT 0
        printf("HLT\n");
        RUN_BIT = 0;
    }
}
