#ifndef INC_SEMAPHORE_H_
#define INC_SEMAPHORE_H_

#include <stdint.h>

/**
 * Different states a semaphore can be in.
 */
typedef enum semaphoreState
{
    SEMAPHORE_TAKEN,
    SEMAPHORE_FREE
} xSemaphoreState_t;

typedef struct
{
    xSemaphoreState_t state;
} xSemaphore_t;

/**
 * Creates a new semaphore.
 */
xSemaphore_t xSemaphoreCreate(void);

/**
 * Takes a semaphore.
 *
 * If the semaphore is taken, the task will be blocked until the semaphore is given.
 *
 * @param semaphore The semaphore to take.
 * @param delayTicks The amount of ticks to delay the task if the semaphore is taken.
 *
 * @return 1 if the semaphore was taken, 0 if the semaphore was not taken.
 */
uint32_t xSemaphoreTake(xSemaphore_t *semaphore, uint32_t delayTicks);

/**
 * Gives a semaphore.
 *
 * @param semaphore The semaphore to give.
 */
void xSemaphoreGive(xSemaphore_t *semaphore);

#endif // INC_SEMAPHORE_H_