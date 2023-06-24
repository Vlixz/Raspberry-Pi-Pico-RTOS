#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

#include "m0plus.h"

typedef struct
{
    volatile uint32_t csr;
    volatile uint32_t rvr;
    volatile uint32_t cvr;
    volatile uint32_t calib;
} systick_t;

#define m0plus_systick_hw ((systick_t *)(PPB_BASE + M0PLUS_SYST_CSR_OFFSET))

#endif // SYSTICK_H