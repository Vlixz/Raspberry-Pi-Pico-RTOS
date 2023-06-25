#ifndef INC_TASK_H
#define INC_TASK_H

#define PRORITY_IDLE 0
#define PRORITY_LOW 1
#define PRORITY_MEDIUM 2
#define PRORITY_HIGH 3

#include <stdint.h>
#include "semaphore.h"

typedef enum taskState
{
    TASK_STATE_RUNNING,
    TASK_STATE_BLOCKED,
    TASK_STATE_DELAYED
} xTaskState_t;

typedef struct xTaskControlBlock
{
    /**
     * The stack pointer points to the top of the stack.
     *
     * To give us space to store the old context of the task, 16 address spaces are reserved.
     *
     * 0 - 16 : Old context
     * 17 - SIZE : The remaining stack space
     */
    int32_t *stackPointer;

    /**
     * Points to the next task in the linkedList.
     *
     * This task is the defualt selected task to be ran next.
     * If all prorities are the same, this task will be ran.
     */
    volatile struct xTaskControlBlock *next;

    /**
     * The current proirity of the task.
     *
     * PRORITY_IDLE
     * PRORITY_LOW
     * PRORITY_MEDIUM
     * PRORITY_HIGH
     */
    uint32_t priority;

    /**
     * Current state of the task.
     *
     * TASK_STATE_RUNNING: The task is currently running.
     * TASK_STATE_BLOCKED: The task is currently blocked. (waiting for a semaphore, mutex, etc.)
     * TASK_STATE_DELAYED: The task is currently delayed. (waiting for a certain amount of ticks)
     */
    xTaskState_t state;

    /**
     * The amount of ticks the task is delayed for (if delayed).
     */
    uint32_t delayTicks;

    /**
     * The name of the task.
     */
    char *name;

    /**
     * The semaphore the task is waiting for (if blocked).
     */
    volatile xSemaphore_t *semaphore;

} xTaskControlBlock_t;

typedef struct xTaskControlBlock_t *xTaskHandle_t;

extern volatile xTaskControlBlock_t *tcb_current; // Current task running
extern volatile xTaskControlBlock_t *tcb_tail;    // First task in the list
extern volatile xTaskControlBlock_t *tcb_head;    // Last task in the list

void xTaskCreate(void (*task)(), char *name, uint32_t stackSize, uint32_t priority, xTaskHandle_t *handle);
void xTaskDelete();
void xTaskDelay(uint32_t ticks);
void xTaskSetPriority();

#endif // TASK_H