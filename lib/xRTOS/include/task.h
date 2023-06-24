#ifndef TASK_H
#define TASK_H

#define PRORITY_IDLE 0
#define PRORITY_LOW 1
#define PRORITY_MEDIUM 2
#define PRORITY_HIGH 3

#include <stdint.h>

typedef struct xTaskControlBlock
{
    int32_t *stackPointer;
    struct xTaskControlBlock *nextStackPointer;
    uint32_t priority;
    uint32_t state;
    char *name;
} xTaskControlBlock_t;

typedef struct xTaskControlBlock_t *xTaskHandle_t;

void xStartSchedular();

void xTaskCreate(void (*task)(), char *name, uint32_t stackSize, uint32_t priority, xTaskHandle_t *handle);
void xTaskDelete();
void xTaskSetPriority();

#endif // TASK_H