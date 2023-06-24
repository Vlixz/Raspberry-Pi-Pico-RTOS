#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define HEAP_SIZE 1024

struct xTaskHandle
{
    uint32_t stackPointer;
    uint32_t stackSize;
    uint32_t priority;
    uint32_t state;
    uint32_t delay;
    uint32_t *stack;
    char *name;
};
typedef struct xTaskHandle xTaskHandle_t;

struct xTaskControlBlock
{
    int32_t *stackPointer;
    struct xTaskControlBlock *nextStackPointer;
};
typedef struct xTaskControlBlock xTaskControlBlock_t;

#define SAVE_CONTEXT()                                                  \
    __asm__ volatile("CPSID     I                   ;"                  \
                     "PUSH      {R4-R7}             ;"                  \
                     "MOV       R4,         R8      ;"                  \
                     "MOV       R5,         R9      ;"                  \
                     "MOV       R6,         R10     ;"                  \
                     "MOV       R7,         R11     ;"                  \
                     "PUSH      {R4-R7}             ;"                  \
                     "LDR       R0,         =currentTaskControlBlock ;" \
                     "LDR       R1,         [R0]    ;"                  \
                     "MOV       R4,         SP      ;"                  \
                     "STR       R4,         [R1]    ;");

#define LOAD_CONTEXT()                                 \
    __asm__ volatile("LDR       R1,         [R1,#4] ;" \
                     "STR       R1,         [R0]    ;" \
                     "LDR       R4,         [R1]    ;" \
                     "MOV       SP,         R4      ;" \
                     "POP       {R4-R7}             ;" \
                     "MOV       R8,         R4      ;" \
                     "MOV       R9,         R5      ;" \
                     "MOV       R10,        R6      ;" \
                     "MOV       R11,        R7      ;" \
                     "POP       {R4-R7}             ;" \
                     "CPSIE     I                   ;" \
                     "BX        LR                  ;");

#endif // COMMON_H