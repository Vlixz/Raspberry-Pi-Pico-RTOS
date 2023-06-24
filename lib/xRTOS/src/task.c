#include "task.h"
#include "common.h"

#include <stdint.h>
#include <stdlib.h>

#include "../lib/m0plus/scb.h"
#include "../lib/m0plus/systick.h"

int32_t RTOS_HEAP[HEAP_SIZE];
int32_t HEAP_INDEX = 0;

xTaskControlBlock_t *currentTaskControlBlock = NULL;
xTaskControlBlock_t *firstTaskControlBlock = NULL;
xTaskControlBlock_t *lastTaskControlBlock = NULL;

void xTaskCreate(void (*task)(), char *name, uint32_t stackSize, uint32_t priority)
{
    HEAP_INDEX += stackSize;

    xTaskControlBlock_t *tcb = (xTaskControlBlock_t *)malloc(sizeof(xTaskControlBlock_t));
    tcb->stackPointer = &RTOS_HEAP[HEAP_INDEX - 16];

    RTOS_HEAP[HEAP_INDEX - 1] = 0x01000000;
    RTOS_HEAP[HEAP_INDEX - 2] = (int32_t)task;

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
