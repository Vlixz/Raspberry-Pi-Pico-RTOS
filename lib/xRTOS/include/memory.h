#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

int get_task_memory(uint32_t size, uint32_t **start_of_stack_pointer);

#endif // MEMORY_H