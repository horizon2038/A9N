ENTRY(kernel_entry)

KERNEL_VMA = 0xFFFF800000000000;
KERNEL_LMA = 0x100000;

SECTIONS
{
    . = KERNEL_VMA + KERNEL_LMA;

    .text ALIGN(0x1000) : AT(ALIGN(0x1000) + KERNEL_LMA)
    {
        __kernel_start = . - KERNEL_VMA;
        *(.text)
        *(.text.*)
    }

    .rodata ALIGN(0x1000) : AT(ALIGN(0x1000) + KERNEL_LMA)
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(0x1000) : AT(ALIGN(0x1000) + KERNEL_LMA)
    {
        *(.data)
        *(.data.*)
    }

    .bss ALIGN(0x1000) : AT(ALIGN(0x1000) + KERNEL_LMA)
    {
        *(.bss)
        *(.bss.*)
        __kernel_end = . - KERNEL_VMA;
    }
}
