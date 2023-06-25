#ifndef INC_COMMON_H
#define INC_COMMON_H

#include <stdint.h>

#define HEAP_SIZE 1024

#define DISABLE_INTERRUPTS() __asm__ volatile("CPSID I");
#define ENABLE_INTERRUPTS() __asm__ volatile("CPSIE I");

#define LAUNCH_SCHEDULAR()                                                 \
    __asm__ volatile("LDR       R0,         =tcb_current                ;" \
                     "LDR       R2,         [R0]                        ;" \
                     "LDR       R4,         [R2]                        ;" \
                     "MOV       SP,         R4                          ;" \
                     "POP       {R4-R7}                                 ;" \
                     "MOV       R8,         R4                          ;" \
                     "MOV       R9,         R5                          ;" \
                     "MOV       R10,        R6                          ;" \
                     "MOV       R11,        R7                          ;" \
                     "POP       {R4-R7}                                 ;" \
                     "POP       {R0-R3}                                 ;" \
                     "POP       {R4}                                    ;" \
                     "MOV       R12,        R4                          ;" \
                     "ADD       SP,         SP,         #4              ;" \
                     "POP       {R4}                                    ;" \
                     "MOV       LR,         R4                          ;" \
                     "ADD       SP,         SP,         #4              ;" \
                     "CPSIE     I                                       ;" \
                     "BX        LR                                      ;");

// Processor already pushed R0, R1, R2, R3, R12, LR, PC, xPSR
// We need to push R4, R5, R6, R7, R8, R9, R10, R11
// We also need to push the stack pointer to the task control block
#define SAVE_CONTEXT()                                      \
    __asm__ volatile("CPSID     I                   ;"      \
                     "PUSH      {R4-R7}             ;"      \
                     "MOV       R4,         R8      ;"      \
                     "MOV       R5,         R9      ;"      \
                     "MOV       R6,         R10     ;"      \
                     "MOV       R7,         R11     ;"      \
                     "PUSH      {R4-R7}             ;"      \
                     "LDR       R0,         =tcb_current ;" \
                     "LDR       R1,         [R0]    ;"      \
                     "MOV       R4,         SP      ;"      \
                     "STR       R4,         [R1]    ;");

// Load the context of the next task
#define LOAD_CONTEXT()                                     \
    __asm__ volatile("LDR       R0,         =tcb_current;" \
                     "LDR       R2,         [R0]        ;" \
                     "LDR       R4,         [R2]        ;" \
                     "MOV       SP,         R4          ;" \
                     "POP       {R4-R7}                 ;" \
                     "MOV       R8,         R4          ;" \
                     "MOV       R9,         R5          ;" \
                     "MOV       R10,        R6          ;" \
                     "MOV       R11,        R7          ;" \
                     "POP       {R4-R7}                 ;" \
                     "CPSIE     I                       ;" \
                     "BX        LR                      ;");

#endif // INC_COMMON_H