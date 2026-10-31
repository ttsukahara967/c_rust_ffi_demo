//! Number formatting for the C firmware, written in Rust and called through a C ABI.
//!
//! The `extern "C"` functions are the only unsafe boundary: they turn the raw
//! pointer + length from C into a slice, and everything after that is safe Rust
//! with bounds checks.

#![cfg_attr(not(test), no_std)]

/// Writes `v` in decimal into `out`. Returns the number of bytes written,
/// or `None` if `out` is too small.
pub fn write_u32(mut v: u32, out: &mut [u8]) -> Option<usize> {
    // Digits are produced in reverse. Iterating over `tmp` (instead of `tmp[n]`) and
    // using `get` lets the compiler see no index can go out of bounds, so no
    // panic path (and none of core's panic formatting code) ends up in the firmware.
    let mut tmp = [0u8; 10];
    let mut n = 0;
    for slot in tmp.iter_mut() {
        *slot = b'0' + (v % 10) as u8;
        n += 1;
        v /= 10;
        if v == 0 {
            break;
        }
    }
    let digits = tmp.get(..n)?;
    let dst = out.get_mut(..n)?;
    for (d, s) in dst.iter_mut().zip(digits.iter().rev()) {
        *d = *s;
    }
    Some(n)
}

/// Writes `v` as `0x` followed by 8 lowercase hex digits into `out`.
/// Returns the number of bytes written, or `None` if `out` is too small.
pub fn write_hex32(v: u32, out: &mut [u8]) -> Option<usize> {
    const DIGITS: &[u8; 16] = b"0123456789abcdef";
    let dst = out.get_mut(..10)?;
    dst[0] = b'0';
    dst[1] = b'x';
    for (i, d) in dst[2..].iter_mut().enumerate() {
        *d = DIGITS[((v >> (28 - 4 * i)) & 0xF) as usize];
    }
    Some(10)
}

/// Converts the C side's pointer + length into a slice, or `None` for a null pointer.
///
/// # Safety
/// `buf` must be null or point to `len` writable bytes that nothing else accesses during the call.
unsafe fn slice_from_c<'a>(buf: *mut u8, len: usize) -> Option<&'a mut [u8]> {
    if buf.is_null() {
        None
    } else {
        Some(unsafe { core::slice::from_raw_parts_mut(buf, len) })
    }
}

/// C: `size_t rs_fmt_u32(uint32_t v, char *buf, size_t len);` Returns 0 on error.
///
/// # Safety
/// `buf` must be null or point to `len` writable bytes.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn rs_fmt_u32(v: u32, buf: *mut u8, len: usize) -> usize {
    unsafe { slice_from_c(buf, len) }
        .and_then(|out| write_u32(v, out))
        .unwrap_or(0)
}

/// C: `size_t rs_fmt_hex32(uint32_t v, char *buf, size_t len);` Returns 0 on error.
///
/// # Safety
/// `buf` must be null or point to `len` writable bytes.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn rs_fmt_hex32(v: u32, buf: *mut u8, len: usize) -> usize {
    unsafe { slice_from_c(buf, len) }
        .and_then(|out| write_hex32(v, out))
        .unwrap_or(0)
}

// A panic must never unwind into C. None is expected here, but the firmware needs a handler.
#[cfg(not(test))]
#[panic_handler]
fn panic(_: &core::panic::PanicInfo) -> ! {
    loop {}
}

#[cfg(test)]
mod tests {
    use super::*;

    fn fmt_u32(v: u32) -> String {
        let mut buf = [0u8; 10];
        let n = write_u32(v, &mut buf).unwrap();
        String::from_utf8(buf[..n].to_vec()).unwrap()
    }

    #[test]
    fn decimal() {
        assert_eq!(fmt_u32(0), "0");
        assert_eq!(fmt_u32(5), "5");
        assert_eq!(fmt_u32(1234567890), "1234567890");
        assert_eq!(fmt_u32(u32::MAX), "4294967295");
    }

    #[test]
    fn hex() {
        let mut buf = [0u8; 10];
        assert_eq!(write_hex32(0x410f_c231, &mut buf), Some(10));
        assert_eq!(&buf, b"0x410fc231");
    }

    #[test]
    fn too_small_buffer_is_rejected() {
        let mut buf = [0u8; 3];
        assert_eq!(write_u32(12345, &mut buf), None);
        assert_eq!(write_hex32(0, &mut buf), None);
        assert_eq!(buf, [0, 0, 0]); // nothing was written past or into it
    }

    #[test]
    fn c_abi_handles_null() {
        assert_eq!(unsafe { rs_fmt_u32(1, core::ptr::null_mut(), 10) }, 0);
    }
}
