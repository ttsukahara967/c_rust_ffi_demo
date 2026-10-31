/* Vector table and reset handler: what cortex-m-rt provides in the Rust version. */

#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss;

int main(void);

void Reset_Handler(void)
{
    /* Copy .data from flash to RAM */
    uint32_t *src = &_sidata;
    for (uint32_t *dst = &_sdata; dst < &_edata;) {
        *dst++ = *src++;
    }

    /* Zero .bss */
    for (uint32_t *dst = &_sbss; dst < &_ebss;) {
        *dst++ = 0;
    }

    main();

    /* main must not return; there is nothing to return to */
    for (;;) {
    }
}

/* Any unexpected exception ends up here (the Rust version's panic-halt plays a similar role) */
void Default_Handler(void)
{
    for (;;) {
    }
}

/* First two words: initial stack pointer and reset vector. The rest are exception handlers. */
__attribute__((section(".vectors"), used))
static const void *const vector_table[16] = {
    &_estack,
    Reset_Handler,
    Default_Handler, /* NMI */
    Default_Handler, /* HardFault */
    Default_Handler, /* MemManage */
    Default_Handler, /* BusFault */
    Default_Handler, /* UsageFault */
    0, 0, 0, 0,      /* Reserved */
    Default_Handler, /* SVCall */
    Default_Handler, /* DebugMonitor */
    0,               /* Reserved */
    Default_Handler, /* PendSV */
    Default_Handler, /* SysTick */
};
