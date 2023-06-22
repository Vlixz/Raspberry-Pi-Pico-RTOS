#ifndef SCHEDULAR_H
#define SCHEDULAR_H

#include <stdint.h>
#include "pico/stdlib.h"

#define NUM_OF_THREADS 2

#define STACKSIZE 500

__attribute__((naked)) void LaunchScheduler();
__attribute__((naked)) void SysTick_Handler(void);

void OsInitThreadStack(volatile void (*func0)(), volatile void (*func1)());

#endif // SCHEDULAR_H