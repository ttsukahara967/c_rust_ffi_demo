# c_rust_ffi_demo

C firmware for an ARM Cortex-M3 that calls a Rust library through the C ABI, run on a Mac via QEMU (no hardware needed).

It is [c_firmware_demo](https://github.com/ttsukahara967/c_firmware_demo) with one piece — number formatting — moved into Rust: the incremental "replace one module at a time" approach used to bring Rust into existing C code bases.
See also the all-Rust version, [rust_firmware_demo](https://github.com/ttsukahara967/rust_firmware_demo).

## Setup

```bash
brew install qemu arm-none-eabi-gcc
rustup target add thumbv7m-none-eabi
```

## Run

```bash
make run    # builds the Rust staticlib, links it into the C firmware, boots it in QEMU
make test   # runs the Rust unit tests natively on the Mac
```

Expected output:

```
Hello from C, formatting numbers in Rust!
SysTick #1
...
SysTick #5
CPUID = 0x410fc231
rs_fmt_u32(12345) into 3 bytes -> 0
```

The last line shows the boundary check: the buffer is too small, so Rust writes nothing and returns 0 instead of overflowing it.

## Layout

| Path | Role |
| --- | --- |
| `rustfmt/src/lib.rs` | `#![no_std]` Rust library. Safe `write_u32` / `write_hex32`, exported to C as `rs_fmt_u32` / `rs_fmt_hex32` |
| `include/rustfmt.h` | C declarations of the Rust functions (hand-written; `cbindgen` can generate it) |
| `src/main.c` | C firmware: SysTick, CPUID, calls into Rust for formatting |
| `src/startup.c`, `src/semihosting.h`, `link.ld` | Same as c_firmware_demo |
| `Makefile` | `cargo build` for the staticlib (`librustfmt_ffi.a`), then `arm-none-eabi-gcc` links it with the C objects |

## How the boundary works

- `#[unsafe(no_mangle)] pub unsafe extern "C" fn` keeps the symbol name and uses the C calling convention, so C sees an ordinary function.
- The only `unsafe` code turns C's pointer + length into a slice (rejecting `NULL`). After that, the formatting is safe Rust with bounds checks.
- The crate is built as `staticlib` (a `.a` the C linker understands) and `rlib` (so `cargo test` can link it). It is `no_std` with its own `#[panic_handler]`, since a panic must never unwind into C.
- `write_u32` iterates instead of indexing with `tmp[n]`. With indexing, the compiler could not prove the index stays in range and linked in Rust's panic and formatting code, which made the image 6.6 KB instead of about 780 bytes (`make size`; the pure C version is about 470 bytes).
