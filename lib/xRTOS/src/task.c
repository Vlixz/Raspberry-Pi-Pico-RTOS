#include "task.h"
#include "memory.h"
#include "common.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "../lib/m0plus/scb.h"
#include "../lib/m0plus/systick.h"

xTaskControlBlock_t *currentTaskControlBlock = NULL;
xTaskControlBlock_t *firstTaskControlBlock = NULL;
xTaskControlBlock_t *lastTaskControlBlock = NULL;

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

    // Thumb bit must be set for the stacked PC
    start_of_stack_address[stackSize - 1] = 0x01000000;
    start_of_stack_address[stackSize - 2] = (int32_t)task;

    // Set the stacked LR to point to the launch scheduler routine (round robin)
    if (firstTaskControlBlock == NULL) // first task created
    {
        lastTaskControlBlock = tcb;
        firstTaskControlBlock = tcb;
        firstTaskControlBlock->nextStackPointer = tcb;
    }
    else // not the first task created
    {
        lastTaskControlBlock->nextStackPointer = tcb;
        tcb->nextStackPointer = firstTaskControlBlock;
        lastTaskControlBlock = tcb;
    }
}

__attribute__((naked)) void SysTick_Handler(void)
{
    SAVE_CONTEXT();
    LOAD_CONTEXT();
}

void xStartSchedular()
{
    // ENABLE SYSTICK ISR   (1ms context switching)
    m0plus_systick_hw->csr = 0;          // Disable systick
    m0plus_systick_hw->rvr = 999ul;      // Set reload value
    m0plus_systick_hw->cvr = 0;          // Clear current value
    m0plus_systick_hw->csr = 0x00000007; // Enable systick, enable interrupts, use processor clock

    // Set systick interrupt handler
    ((volatile uint32_t *)m0plus_scb_hw->vtor)[15] = (uint32_t)SysTick_Handler;

    // Setup the first task to be ran
    currentTaskControlBlock = firstTaskControlBlock;

    __asm("CPSID   I"); // disable interrupts

    LAUNCH_SCHEDULAR();
}
