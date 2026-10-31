/* C declarations of the functions implemented in Rust (rustfmt/src/lib.rs).
 * Written by hand to match; cbindgen can generate this from the Rust source. */

#ifndef RUSTFMT_H
#define RUSTFMT_H

#include <stddef.h>
#include <stdint.h>

/* Write v in decimal into buf (no NUL). Returns bytes written, or 0 if buf is NULL or too small. */
size_t rs_fmt_u32(uint32_t v, char *buf, size_t len);

/* Write v as "0x" + 8 hex digits into buf (no NUL). Returns 10, or 0 if buf is NULL or too small. */
size_t rs_fmt_hex32(uint32_t v, char *buf, size_t len);

#endif
