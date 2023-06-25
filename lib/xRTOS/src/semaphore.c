#include "task.h"
#include "common.h"

xSemaphore_t xSemaphoreCreate(void)
{
    xSemaphore_t semaphore;

    semaphore.state = SEMAPHORE_FREE;

    return semaphore;
}

uint32_t xSemaphoreTake(xSemaphore_t *semaphore, uint32_t delayTicks)
{
    DISABLE_INTERRUPTS();

    if (semaphore->state == SEMAPHORE_FREE)
    {
        semaphore->state = SEMAPHORE_TAKEN;
        return 1;
    }

    // Add current task to semaphore's waiting list
    tcb_current->semaphore = semaphore;
    tcb_current->state = TASK_STATE_WAITING_FOR_SEMAPHORE;
    tcb_current->delayTicks = delayTicks;

    ENABLE_INTERRUPTS();

    while (tcb_current->state == TASK_STATE_WAITING_FOR_SEMAPHORE)
        __asm("WFI"); // Wait for interrupt

    if (tcb_current->semaphore->state == SEMAPHORE_FREE)
        return 1; // Semaphore was given

    return 0; // Semaphore was not given (timed out)
}

void xSemaphoreGive(xSemaphore_t *semaphore)
{
    DISABLE_INTERRUPTS();

    semaphore->state = SEMAPHORE_FREE;

    ENABLE_INTERRUPTS();
}