#include "task.h"
#include "common.h"

#include <stdint.h>
#include <stdlib.h>

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
    // hw_clear_bits(&timer_hw->intr, 1u << TIMER_IRQ_0);
    // timer_hw->alarm[TIMER_IRQ_0] = (uint32_t)timer_hw->timerawl + 10000;

    SAVE_CONTEXT();
    LOAD_CONTEXT();
}

void xStartSchedular()
{
    currentTaskControlBlock = firstTaskControlBlock;

    // hw_set_bits(&timer_hw->inte, 1u << TIMER_IRQ_0);

    // irq_set_exclusive_handler(TIMER_IRQ_0, SysTick_Handler);

    // irq_set_enabled(TIMER_IRQ_0, true);

    // uint64_t target = timer_hw->timerawl + 1000; // Every 1ms

    // timer_hw->alarm[TIMER_IRQ_0] = (uint32_t)target;

    __asm("CPSID   I"); // disable interrupts

    LaunchScheduler();
}
