#ifndef INC_SEMAPHORE_H_
#define INC_SEMAPHORE_H_

#include <stdint.h>

typedef enum semaphoreState
{
    SEMAPHORE_TAKEN,
    SEMAPHORE_FREE
} xSemaphoreState_t;

typedef struct
{
    xSemaphoreState_t state;
} xSemaphore_t;

xSemaphore_t xSemaphoreCreate(void);

uint32_t xSemaphoreTake(xSemaphore_t *semaphore, uint32_t delayTicks);
void xSemaphoreGive(xSemaphore_t *semaphore);

#endif // INC_SEMAPHORE_H_