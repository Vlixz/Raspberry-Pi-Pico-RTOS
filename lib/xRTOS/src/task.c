#include "xRTOS.h"

#include "task.h"
#include "memory.h"
#include "common.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

volatile xTaskControlBlock_t *tcb_current = NULL; // Current task running
volatile xTaskControlBlock_t *tcb_tail = NULL;    // First task in the list
volatile xTaskControlBlock_t *tcb_head = NULL;    // Last task in the list

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

void xTaskDelay(uint32_t ticks)
{
    DISABLE_INTERRUPTS();

    tcb_current->delayTicks = ticks;
    tcb_current->state = TASK_STATE_DELAYED;

    ENABLE_INTERRUPTS();

    while (tcb_current->state == TASK_STATE_DELAYED)
        asm volatile(""); // wait for the schedular to be called
}
