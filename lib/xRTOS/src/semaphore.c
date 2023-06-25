#include "task.h"

xSemaphore_t xSemaphoreCreate(void)
{
    xSemaphore_t semaphore;

    semaphore.state = SEMAPHORE_FREE;

    return semaphore;
}

uint32_t xSemaphoreTake(xSemaphore_t *semaphore, uint32_t delayTicks)
{
    __asm("CPSID   I"); // Disable interrupts

    if (semaphore->state == SEMAPHORE_FREE)
    {
        semaphore->state = SEMAPHORE_TAKEN;
        return 1; // Semaphore was taken
    }

    // Add current task to semaphore's waiting list
    tcb_current->semaphore = semaphore;
    tcb_current->state = TASK_STATE_BLOCKED;
    tcb_current->delayTicks = delayTicks;

    __asm("CPSIE   I"); // Enable interrupts

    while (tcb_current->state == TASK_STATE_BLOCKED)
        __asm("WFI"); // Wait for interrupt

    if (tcb_current->semaphore->state == SEMAPHORE_FREE)
        return 1; // Semaphore was taken
    else
        return 0; // Semaphore was not taken (timed out)
}

void xSemaphoreGive(xSemaphore_t *semaphore)
{
    __asm("CPSID   I"); // Disable interrupts

    semaphore->state = SEMAPHORE_FREE;

    __asm("CPSIE   I"); // Enable interrupts
}