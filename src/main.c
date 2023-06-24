#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <task.h>

#include "pico/stdlib.h"
#include "hardware/uart.h"

#define SYS_SECOND 48000000

#define UART_ID uart0
#define BAUD_RATE 115200

#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define SYST_CRS 0xe010
#define SYST_RVR 0xe014

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
    portable_delay_s(1);
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
    portable_delay_s(2);

    count++;

    char str[10];
    sprintf(str, "Task 1: %d\n", count);

    uart_puts(UART_ID, str);
  }
}

volatile void Task2()
{
  int count = 0;

  while (1)
  {
    portable_delay_s(3);

    count++;

    char str[10];
    sprintf(str, "Task 2: %d\n", count);

    uart_puts(UART_ID, str);
  }
}

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
  enable_uart();

  xTaskCreate(Task0, "Task 0", 200, PRORITY_LOW);
  xTaskCreate(Task1, "Task 1", 200, PRORITY_MEDIUM);
  xTaskCreate(Task2, "Task 2", 200, PRORITY_HIGH);

  xStartSchedular();

  while (1)
    ;
}