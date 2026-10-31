/* Memory layout of the TI LM3S6965 emulated by QEMU (lm3s6965evb) */
MEMORY
{
  FLASH : ORIGIN = 0x00000000, LENGTH = 256K
  RAM   : ORIGIN = 0x20000000, LENGTH = 64K
}

ENTRY(Reset_Handler)

/* Stack grows down from the top of RAM */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
  .text :
  {
    KEEP(*(.vectors)) /* Vector table must be at address 0 */
    *(.text*)
    *(.rodata*)
  } > FLASH

  /* Initialized variables: stored in flash, copied to RAM at reset */
  .data :
  {
    _sdata = .;
    *(.data*)
    _edata = .;
  } > RAM AT > FLASH
  _sidata = LOADADDR(.data);

  /* Zero-initialized variables: cleared at reset */
  .bss (NOLOAD) :
  {
    _sbss = .;
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } > RAM
}
