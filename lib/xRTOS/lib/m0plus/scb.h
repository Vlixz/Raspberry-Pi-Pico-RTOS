#ifndef SCB_H
#define SCB_H

#include <stdint.h>

#include "m0plus.h"

typedef struct
{
    volatile uint32_t cpuid;
    volatile uint32_t icsr;
    volatile uint32_t vtor;
    volatile uint32_t aircr;
    volatile uint32_t scr;
} scb_t;

#define m0plus_scb_hw ((scb_t *)(PPB_BASE + M0PLUS_CPUID_OFFSET))

#endif // SCB_H
