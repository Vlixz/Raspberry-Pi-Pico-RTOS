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

#endif // COMMON_H