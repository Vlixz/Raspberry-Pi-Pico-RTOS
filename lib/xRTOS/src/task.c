#include "task.h"
#include "common.h"

#include <stdint.h>
#include <stdlib.h>

#include "hardware/irq.h"
#include "hardware/exception.h"
#include "hardware/structs/scb.h"
#include "hardware/structs/systick.h"

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

    // 6. Set the stacked LR to point to the launch scheduler routine (round robin)
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

__attribute__((naked)) void LaunchScheduler(void)
{
    /// R0 contains the address of currentPt
    __asm("LDR     R0, =currentTaskControlBlock");

    /// R2 contains the address in currentPt(value of currentPt)
    __asm("LDR     R2, [R0]");

    /// Load the SP reg with the stacked SP value
    __asm("LDR     R4, [R2]");
    __asm("MOV     SP, R4");

    /// Pop registers R8-R11(user saved context)
    __asm("POP     {R4-R7}");
    __asm("MOV     R8, R4");
    __asm("MOV     R9, R5");
    __asm("MOV     R10, R6");
    __asm("MOV     R11, R7");

    /// Pop registers R4-R7(user saved context)
    __asm("POP     {R4-R7}");
    ///  Start poping the stacked exception frame.
    __asm("POP     {R0-R3}");
    __asm("POP     {R4}");
    __asm("MOV     R12, R4");

    /// Skip the saved LR
    __asm("ADD     SP,SP,#4");

    /// POP the saved PC into LR via R4, We do this to jump into the
    /// first task when we execute the branch instruction to exit this routine.
    __asm("POP     {R4}");
    __asm("MOV     LR, R4");
    __asm("ADD     SP,SP,#4");

    /// Enable interrupts
    __asm("CPSIE   I ");
    __asm("BX      LR");
}

__attribute__((naked)) void SysTick_Handler(void)
{
    SAVE_CONTEXT();
    LOAD_CONTEXT();
}

void xStartSchedular()
{
    // ENABLE SYSTICK ISR   (1ms context switching)
    systick_hw->csr = 0;          // Disable systick
    systick_hw->rvr = 999ul;      // Set reload value
    systick_hw->cvr = 0;          // Clear current value
    systick_hw->csr = 0x00000007; // Enable systick, enable interrupts, use processor clock

    // Set systick interrupt handler
    ((exception_handler_t *)scb_hw->vtor)[15] = SysTick_Handler;

    // SET UP THE FIRST TASK
    currentTaskControlBlock = firstTaskControlBlock;

    __asm("CPSID   I"); // disable interrupts

    LaunchScheduler();
}
