#include "task.h"
#include "memory.h"
#include "common.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "../lib/m0plus/scb.h"
#include "../lib/m0plus/systick.h"

volatile xTaskControlBlock_t *tcb_current = NULL;
volatile xTaskControlBlock_t *tcb_tail = NULL;
volatile xTaskControlBlock_t *tcb_head = NULL;

void xTaskCreate(void (*task)(), char *name, uint32_t stackSize, uint32_t priority, xTaskHandle_t *handle)
{
    uint32_t *start_of_stack_address;

    if (!get_task_memory(stackSize, &start_of_stack_address))
    {
        return; // not enough memory
    }

    xTaskControlBlock_t *tcb = (xTaskControlBlock_t *)*handle;

    tcb = (xTaskControlBlock_t *)malloc(sizeof(xTaskControlBlock_t));

    tcb->stackPointer = &start_of_stack_address[stackSize - 16];

    tcb->name = malloc(sizeof(char) * strlen(name));
    strcpy(tcb->name, name);

    tcb->priority = priority;
    tcb->state = TASK_STATE_RUNNING;

    // Thumb bit must be set for the stacked PC
    start_of_stack_address[stackSize - 1] = 0x01000000;
    start_of_stack_address[stackSize - 2] = (int32_t)task;

    // Set the stacked LR to point to the launch scheduler routine (round robin)
    if (tcb_tail == NULL) // first task created
    {
        tcb_head = tcb;
        tcb_tail = tcb;
        tcb_tail->next = tcb;
    }
    else // not the first task created
    {
        tcb_head->next = tcb;
        tcb->next = tcb_tail;
        tcb_head = tcb;
    }
}

__attribute__((naked)) void SysTick_Handler(void)
{
    SAVE_CONTEXT();

    // TODO: Implement round robin scheduling
    // TODO: Implement task delay

    volatile xTaskControlBlock_t *tcb_pivot = tcb_current;
    volatile xTaskControlBlock_t *tcb_next = tcb_current->next;

    do
    {
        switch (tcb_pivot->state)
        {
        case TASK_STATE_DELAYED:
            if (tcb_pivot->delayTicks > 0)
                tcb_pivot->delayTicks--;
            else
                tcb_pivot->state = TASK_STATE_RUNNING;
            break;

        case TASK_STATE_BLOCKED:
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

void xTaskDelay(uint32_t ticks)
{
    __asm("CPSID   I"); // disable interrupts

    tcb_current->delayTicks = ticks;
    tcb_current->state = TASK_STATE_DELAYED;

    __asm("CPSIE   I"); // enable interrupts

    while (tcb_current->state == TASK_STATE_DELAYED)
        asm volatile("");
}

void xStartSchedular()
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
