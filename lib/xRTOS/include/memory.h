#ifndef INC_MEMORY_H
#define INC_MEMORY_H

#include <stdint.h>

/**
 * Allocates memory for a task.
 *
 * @param size The size of the memory to allocate.
 * @param start_of_stack_pointer A pointer to the start of the stack pointer.
 *
 * @return 1 if the memory was allocated, 0 if the memory was not allocated.
 */
int get_task_memory(uint32_t size, uint32_t **start_of_stack_pointer);

#endif // INC_MEMORY_H