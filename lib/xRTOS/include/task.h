#ifndef TASK_H
#define TASK_H

#define PRORITY_IDLE 0
#define PRORITY_LOW 1
#define PRORITY_MEDIUM 2
#define PRORITY_HIGH 3

#include <stdint.h>

typedef enum taskState
{
    TASK_STATE_RUNNING,
    TASK_STATE_BLOCKED,
    TASK_STATE_DELAYED
} xTaskState_t;

typedef struct xTaskControlBlock
{
    int32_t *stackPointer;
    struct xTaskControlBlock *next;
    uint32_t priority;
    xTaskState_t state;
    uint32_t delayTicks;
    char *name;
} xTaskControlBlock_t;

typedef struct xTaskControlBlock_t *xTaskHandle_t;

void xStartSchedular();

void xTaskCreate(void (*task)(), char *name, uint32_t stackSize, uint32_t priority, xTaskHandle_t *handle);
void xTaskDelete();
void xTaskDelay(uint32_t ticks);
void xTaskSetPriority();

#endif // TASK_H