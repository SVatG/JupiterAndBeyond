ENTRY(g_pfnVectors)

MEMORY
{
	flash (rx)		: ORIGIN = 0x08000000, LENGTH = 1024K
	ccmram (rwx)	: ORIGIN = 0x10000000, LENGTH = 64K
	sram (rwx)		: ORIGIN = 0x20000000, LENGTH = 128K
}

_estack = 0x10010000;

SECTIONS
{
    .text :
    {
        _text = .;
        __isr_vector_start = .;
        KEEP(*(.isr_vector))
        __isr_vector_end = .;
        *(.text*)
        *(.rodata*)
        _etext = .;
        _sidata = .;
    } > flash
    .data : AT(ADDR(.text) + SIZEOF(.text))
    {
        _sdata = .;
        *(vtable)
        *(.data*)
        _edata = .;
    } > ccmram
    .bss :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > ccmram
}

