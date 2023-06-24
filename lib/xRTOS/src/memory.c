#include "memory.h"
#include "common.h"

int32_t heap[HEAP_SIZE];
int32_t HEAP_INDEX = 0;

// Returns 0 if there is not enough memory left in the heap
// Returns 1 if there is enough memory left in the heap
// Returns the end of the stack in the start_of_stack pointer
int get_task_memory(uint32_t size, uint32_t **start_of_stack_pointer)
{
    // Check if there is enough memory left in the heap
    if (HEAP_INDEX + size > HEAP_SIZE)
    {
        return 0;
    }

    *start_of_stack_pointer = &heap[HEAP_INDEX];
    HEAP_INDEX += size;

    return 1;
}