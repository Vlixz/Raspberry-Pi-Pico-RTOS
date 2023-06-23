#ifndef TASK_H
#define TASK_H

#define PRORITY_IDLE 0
#define PRORITY_LOW 1
#define PRORITY_MEDIUM 2
#define PRORITY_HIGH 3

#include <stdint.h>

void xStartSchedular();

void xTaskCreate(void (*task)(), char *name, uint32_t stackSize, uint32_t priority);
void xTaskDelete();
void xTaskSetPriority();

#endif // TASK_H