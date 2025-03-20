#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"

void process_instruction () {
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. 
     * */
}

void ADDS (void* rn, int64_t* x0, int64_t* x1) {
    CURRENT_STATE.REGS[*x0] = CURRENT_STATE.REGS[*x1] + CURRENT_STATE.REGS[*(int*)rn];
    if (CURRENT_STATE.REGS[*x0] < 0){
        CURRENT_STATE.FLAG_N = 01;
    }
    else if (CURRENT_STATE.REGS[*x0] == 0){
        CURRENT_STATE.FLAG_Z = 01;
    }
    else {
        CURRENT_STATE.FLAG_N = 00;
        CURRENT_STATE.FLAG_Z = 00;
    }
}

void ADDS (void* rn, int64_t* x0, int64_t* x1) {
    int64_t operand;

    // Decodificar si rn es un registro o un valor inmediato
    if (is_register(rn)) { // Supongamos que tienes una función is_register()
        operand = CURRENT_STATE.REGS[*(int*)rn]; // Es un registro
    } else {
        operand = *(int64_t*)rn; // Es un valor inmediato
    }

    CURRENT_STATE.REGS[*x0] = CURRENT_STATE.REGS[*x1] + operand;

    // Actualizar banderas
    if (CURRENT_STATE.REGS[*x0] < 0) {
        CURRENT_STATE.FLAG_N = 01;
    } else if (CURRENT_STATE.REGS[*x0] == 0) {
        CURRENT_STATE.FLAG_Z = 01;
    } else {
        CURRENT_STATE.FLAG_N = 00;
        CURRENT_STATE.FLAG_Z = 00;
    }
}