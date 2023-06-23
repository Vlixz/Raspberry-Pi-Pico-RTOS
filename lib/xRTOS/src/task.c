#include "task.h"
#include "common.h"

#include <stdint.h>
#include <stdlib.h>

#include "pico/stdlib.h"

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
    hw_clear_bits(&timer_hw->intr, 1u << TIMER_IRQ_0);
    timer_hw->alarm[TIMER_IRQ_0] = (uint32_t)timer_hw->timerawl + 10000;

    /// STEP 1 - SAVE THE CURRENT TASK CONTEXT

    /// At this point the processor has already pushed PSR, PC, LR, R12, R3, R2, R1 and R0
    /// onto the stack. We need to push the rest(i.e R4, R5, R6, R7, R8, R9, R10 & R11) to save the
    /// context of the current task.
    /// Disable interrupts
    __asm("CPSID   I");

    /// Push registers R4 to R7
    __asm("PUSH    {R4-R7}");

    /// Push registers R8-R11
    __asm("MOV     R4, R8");
    __asm("MOV     R5, R9");
    __asm("MOV     R6, R10");
    __asm("MOV     R7, R11");
    __asm("PUSH    {R4-R7}");

    /// Load R0 with the address of pCurntTcb
    __asm("LDR     R0, =currentTaskControlBlock");

    /// Load R1 with the content of pCurntTcb(i.e post this, R1 will contain the address of current TCB).
    __asm("LDR     R1, [R0]");

    /// Move the SP value to R4
    __asm("MOV     R4, SP");

    /// Store the value of the stack pointer(copied in R4) to the current tasks "stackPt" element in its TCB.
    /// This marks an end to saving the context of the current task.
    __asm("STR     R4, [R1]");

    /// STEP 2: LOAD THE NEW TASK CONTEXT FROM ITS STACK TO THE CPU REGISTERS, UPDATE pCurntTcb.

    /// Load the address of the next task TCB onto the R1.
    __asm("LDR     R1, [R1,#4]");

    /// Load the contents of the next tasks stack pointer to pCurntTcb, equivalent to pointing pCurntTcb to
    /// the newer tasks TCB. Remember R1 contains the address of pCurntTcb.
    __asm("STR     R1, [R0]");

    /// Load the newer tasks TCB to the SP using R4.
    __asm("LDR     R4, [R1]");
    __asm("MOV     SP, R4");

    /// Pop registers R8-R11
    __asm("POP     {R4-R7}");
    __asm("MOV     R8, R4");
    __asm("MOV     R9, R5");
    __asm("MOV     R10, R6");
    __asm("MOV     R11, R7");

    /// Pop registers R4-R7
    __asm("POP     {R4-R7}");
    __asm("CPSIE   I ");
    __asm("BX      LR");
}

void xStartSchedular()
{
    currentTaskControlBlock = firstTaskControlBlock;

    hw_set_bits(&timer_hw->inte, 1u << TIMER_IRQ_0);

    irq_set_exclusive_handler(TIMER_IRQ_0, SysTick_Handler);

    irq_set_enabled(TIMER_IRQ_0, true);

    uint64_t target = timer_hw->timerawl + 1000; // Every 1ms

    timer_hw->alarm[TIMER_IRQ_0] = (uint32_t)target;

    __asm("CPSID   I"); // disable interrupts

    LaunchScheduler();
}
