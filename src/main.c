/* C firmware that calls into Rust for number formatting (see include/rustfmt.h). */

#include <stdint.h>

#include "rustfmt.h"
#include "semihosting.h"

#define SYST_CSR (*(volatile uint32_t *)0xE000E010)
#define SYST_RVR (*(volatile uint32_t *)0xE000E014)
#define SYST_CVR (*(volatile uint32_t *)0xE000E018)
#define CPUID (*(volatile const uint32_t *)0xE000ED00)

#define SYST_CSR_ENABLE (1u << 0)
#define SYST_CSR_CLKSOURCE (1u << 2)
#define SYST_CSR_COUNTFLAG (1u << 16)

static char *copy(char *p, const char *s)
{
    while (*s != '\0') {
        *p++ = *s++;
    }
    return p;
}

int main(void)
{
    char buf[32];
    char *p;

    sh_puts("Hello from C, formatting numbers in Rust!\n");

    SYST_RVR = 12000000 / 10; /* ~100 ms at 12 MHz */
    SYST_CVR = 0;
    SYST_CSR = SYST_CSR_CLKSOURCE | SYST_CSR_ENABLE;

    for (uint32_t tick = 1; tick <= 5; tick++) {
        while ((SYST_CSR & SYST_CSR_COUNTFLAG) == 0) {
        }
        p = copy(buf, "SysTick #");
        p += rs_fmt_u32(tick, p, sizeof(buf) - (size_t)(p - buf) - 2); /* leave room for "\n\0" */
        *p++ = '\n';
        *p = '\0';
        sh_puts(buf);
    }

    p = copy(buf, "CPUID = ");
    p += rs_fmt_hex32(CPUID, p, sizeof(buf) - (size_t)(p - buf) - 2);
    *p++ = '\n';
    *p = '\0';
    sh_puts(buf);

    /* A buffer that is too small: Rust refuses to write instead of overflowing it */
    char small[3] = {'-', '-', '-'};
    size_t n = rs_fmt_u32(12345, small, sizeof(small));
    p = copy(buf, "rs_fmt_u32(12345) into 3 bytes -> ");
    p += rs_fmt_u32((uint32_t)n, p, sizeof(buf) - (size_t)(p - buf) - 2);
    *p++ = '\n';
    *p = '\0';
    sh_puts(buf);

    sh_exit_success();
    for (;;) {
    }
}
