#ifndef M0PLUS_H
#define M0PLUS_H

// Cortex-M0+ Processor Core register base address
#define PPB_BASE 0xe0000000

// SYSTICK
#define M0PLUS_SYST_CSR_OFFSET 0x0000e010   // SysTick Control and Status Register
#define M0PLUS_SYST_RVR_OFFSET 0x0000e014   // SysTick Reload Value Register
#define M0PLUS_SYST_CVR_OFFSET 0x0000e018   // SysTick Current Value Register
#define M0PLUS_SYST_CALIB_OFFSET 0x0000e01c // SysTick Calibration Value Register

// SCB
#define M0PLUS_CPUID_OFFSET 0x0000ed00 // CPUID Base Register
#define M0PLUS_ICSR_OFFSET 0x0000ed04  // Interrupt Control and State Register
#define M0PLUS_VTOR_OFFSET 0x0000ed08  // Vector Table Offset Register
#define M0PLUS_AIRCR_OFFSET 0x0000ed0c // Application Interrupt and Reset Control Register
#define M0PLUS_SCR_OFFSET 0x0000ed10   // System Control Register

#endif // M0PLUS_H