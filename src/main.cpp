/*
 * premptive_scheduler.c
 *
 *  Created on: Apr 26, 2020
 *      Author: Sudeep Chandrasekaran
 */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define NUM_OF_THREADS 2

#define SYS_SECOND 48000000

#define ALARM_NUM 0
#define ALARM_IRQ TIMER_IRQ_0

#define UART_ID uart0
#define BAUD_RATE 115200

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

/// Task control block, implemented as a linked list to point to the
/// TCB of the next task.
struct tcb
{
  int32_t *stackPt;
  struct tcb *nextPt;
};
/// Define tcb_t datatype
typedef struct tcb tcb_t;
/// Define an array to store TCB's for the tasks.
tcb_t tcbs[NUM_OF_THREADS];
/// Points to the TCB of the currently active task
tcb_t *pCurntTcb;

#define STACKSIZE 500
/// Define stack for each task. Node that the processor expects the stacks
/// to be ended on word boundaries.
int32_t TCB_STACK[NUM_OF_THREADS][STACKSIZE];

void portable_delay_s(unsigned long n)
{
  int delay = (SYS_SECOND * n) / 2;

  while (delay--)
  {
    asm volatile("");
  }
}

volatile void Task0()
{
  int count = 0;

  while (1)
  {
    sleep_ms(1000);
    count++;

    char str[10];
    sprintf(str, "Task 0: %d\n", count);

    uart_puts(UART_ID, str);
  }
}

volatile void Task1()
{
  int count = 0;

  while (1)
  {
    sleep_ms(2000);

    count++;

    char str[10];
    sprintf(str, "Task 1: %d\n", count);

    uart_puts(UART_ID, str);
  }
}

__attribute__((naked)) void SysTick_Handler(void)
{
  hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM);
  timer_hw->alarm[ALARM_NUM] = (uint32_t)timer_hw->timerawl + 10000;

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
  __asm("LDR     R0, =pCurntTcb");
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

void OsInitThreadStack()
{
  /// Enter critical section
  /// Disable interrupts
  __asm("CPSID   I");

  /// Make the TCB linked list circular
  tcbs[0].nextPt = &tcbs[1];
  tcbs[1].nextPt = &tcbs[0];

  /// Setup stack for task0

  /// Setup the stack such that it is holding one task context.
  /// Remember it is a descending stack and a context consists of 16 registers.

  tcbs[0].stackPt = &TCB_STACK[0][STACKSIZE - 16];
  /// Set the 'T' bit in stacked xPSR to '1' to notify processor
  /// on exception return about the thumb state. V6-m and V7-m cores
  /// can only support thumb state hence this should be always set
  /// to '1'.
  TCB_STACK[0][STACKSIZE - 1] = 0x01000000;

  /// Set the stacked PC to point to the task
  TCB_STACK[0][STACKSIZE - 2] = (int32_t)(Task0);

  /// Setup stack for task1

  /// Setup the stack such that it is holding one task context.
  /// Remember it is a descending stack and a context consists of 16 registers.
  tcbs[1].stackPt = &TCB_STACK[1][STACKSIZE - 16];

  /// Set the 'T' bit in stacked xPSR to '1' to notify processor
  /// on exception return about the thumb state. V6-m and V7-m cores
  /// can only support thumb state hence this should be always set
  /// to '1'.
  TCB_STACK[1][STACKSIZE - 1] = 0x01000000;

  /// Set the stacked PC to point to the task
  TCB_STACK[1][STACKSIZE - 2] = (int32_t)(Task1);

  /// Make current tcb pointer point to task0
  pCurntTcb = &tcbs[0];

  /// Enable interrupts
  __asm("CPSIE   I ");
}

__attribute__((naked)) void LaunchScheduler(void)
{
  /// R0 contains the address of currentPt
  __asm("LDR     R0, =pCurntTcb");
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

#define SYST_CRS 0xe010
#define SYST_RVR 0xe014

void enable_uart()
{
  // Set up our UART with the required speed.
  uart_init(UART_ID, BAUD_RATE);

  // Set the TX and RX pins by using the function select on the GPIO
  // Set datasheet for more information on function select
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

  // Use some the various UART functions to send out data
  // In a default system, printf will also output via the default UART

  // Send out a character without any conversions
  uart_putc_raw(UART_ID, 'A');

  // Send out a character but do CR/LF conversions
  uart_putc(UART_ID, 'B');

  // Send out a string, with CR/LF conversions
  uart_puts(UART_ID, " Hello, UART!\n");
}

int main()
{
  set_sys_clock_48mhz();

  // Enable the interrupt for our alarm (the timer outputs 4 alarm irqs)
  hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM);

  // Set irq handler for alarm irq
  irq_set_exclusive_handler(ALARM_IRQ, SysTick_Handler);

  // Enable the alarm irq
  irq_set_enabled(ALARM_IRQ, true);

  // Enable interrupt in block and at processor

  // Alarm is only 32 bits so if trying to delay more
  // than that need to be careful and keep track of the upper
  // bits
  uint64_t target = timer_hw->timerawl + 1000;

  // Write the lower 32 bits of the target time to the alarm which
  // will arm it
  timer_hw->alarm[ALARM_NUM] = (uint32_t)target;

  enable_uart();

  OsInitThreadStack();
  LaunchScheduler();

  while (1)
    ;
}