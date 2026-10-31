/* Minimal semihosting: what cortex-m-semihosting provides in the Rust version. */

#ifndef SEMIHOSTING_H
#define SEMIHOSTING_H

#include <stdint.h>

#define SYS_OPEN 0x01
#define SYS_WRITE 0x05
#define SYS_EXIT 0x18
#define OPEN_MODE_W 4 /* "w" */
#define ADP_STOPPED_APPLICATION_EXIT 0x20026

/* BKPT 0xAB asks the host (QEMU or a debugger) to perform operation `op` with argument `arg` */
static inline uint32_t semihosting_call(uint32_t op, const void *arg)
{
    register uint32_t r0 __asm__("r0") = op;
    register const void *r1 __asm__("r1") = arg;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    return r0;
}

/* Open the host's stdout (":tt"), the same way hprintln! does in the Rust version */
static inline uint32_t sh_open_stdout(void)
{
    const uint32_t args[3] = {(uint32_t)":tt", OPEN_MODE_W, 3};
    return semihosting_call(SYS_OPEN, args);
}

static inline void sh_puts(const char *s)
{
    static uint32_t handle = (uint32_t)-1;
    if (handle == (uint32_t)-1) {
        handle = sh_open_stdout();
    }
    uint32_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    const uint32_t args[3] = {handle, (uint32_t)s, len};
    semihosting_call(SYS_WRITE, args);
}

static inline void sh_exit_success(void)
{
    semihosting_call(SYS_EXIT, (const void *)ADP_STOPPED_APPLICATION_EXIT);
}

#endif
