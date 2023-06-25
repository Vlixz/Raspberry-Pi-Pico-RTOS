#include "xRTOS.h"
#include "task.h"
#include "common.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "../lib/m0plus/scb.h"
#include "../lib/m0plus/systick.h"

__attribute__((naked)) void SysTick_Handler(void)
{
    SAVE_CONTEXT();

    volatile xTaskControlBlock_t *tcb_pivot = tcb_current;
    volatile xTaskControlBlock_t *tcb_next = tcb_current->next;

    do
    {
        switch (tcb_pivot->state)
        {
        case TASK_STATE_DELAYED:

            if (tcb_pivot->delayTicks > 0)
                tcb_pivot->delayTicks--; // decrement the delay ticks

            if (tcb_pivot->delayTicks == 0)
                tcb_pivot->state = TASK_STATE_RUNNING; // if the delay is over, set the task to running

            break;

        case TASK_STATE_WAITING_FOR_SEMAPHORE:

            if (tcb_pivot->semaphore->state == SEMAPHORE_FREE)
                tcb_pivot->state = TASK_STATE_RUNNING; // if the semaphore is free, set the task to running

            if (tcb_pivot->delayTicks > 0)
                tcb_pivot->delayTicks--; // decrement the delay ticks

            if (tcb_pivot->delayTicks == 0)
                tcb_pivot->state = TASK_STATE_RUNNING; // if the delay is over, set the task to running

            break;
        case TASK_STATE_RUNNING:

            if (tcb_next->state != TASK_STATE_RUNNING)
                tcb_next = tcb_pivot; // if the next task is not running, switch to this task

            if (tcb_pivot->priority >= tcb_next->priority)
                tcb_next = tcb_pivot; // if the priority is higher than the current task, switch to this task

            break;
        default:
            break;
        }

        tcb_pivot = tcb_pivot->next;
    } while (tcb_pivot != tcb_current);

    tcb_current = tcb_next;

    LOAD_CONTEXT();
}

void xStartSchedular(void)
{
    // ENABLE SYSTICK ISR   (1ms context switching)
    m0plus_systick_hw->csr = 0;          // Disable systick
    m0plus_systick_hw->rvr = 125000u;    // Set reload value to 1ms
    m0plus_systick_hw->cvr = 0;          // Clear current value
    m0plus_systick_hw->csr = 0x00000007; // Enable systick, enable interrupts, use processor clock

    // Set systick interrupt handler
    ((volatile uint32_t *)m0plus_scb_hw->vtor)[15] = (uint32_t)SysTick_Handler;

    // Setup the first task to be ran
    tcb_current = tcb_tail;

    __asm("CPSID   I"); // disable interrupts

    LAUNCH_SCHEDULAR();
}
