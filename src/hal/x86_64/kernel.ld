ENTRY(trampoline)

/* KERNEL_VMA = 0xFFFF800000000000; */
KERNEL_VMA = 0xFFFF800000000000;
KERNEL_LMA = 0x100000;

SECTIONS
{
    . = KERNEL_LMA;

    .boot :
    {
        __kernel_start = .;
        KEEP(*(.boot));
        . = ALIGN(4096);
    }

    . = KERNEL_VMA + KERNEL_LMA - .;

    .text : AT(ADDR(.text) - KERNEL_VMA)
    {
        *(.text);
        *(.text.*);
    }

    .rodata : AT(ADDR(.rodata) - KERNEL_VMA)
    {
        *(.rodata);
        *(.rodata.*);
    }

    .data : AT(ADDR(.data) - KERNEL_VMA)
    {
        *(.data);
        *(.data.*);
    }

    .bss : AT(ADDR(.bss) - KERNEL_VMA)
    {
        __bss_start = .;
        *(.bss);
        *(.bss.*);
        *(COMMON);
        __bss_end = .;

        . = ALIGN(4096);
        __kernel_pml4 = . - KERNEL_LMA;
        . += 0x1000;

        __kernel_pdpt = . - KERNEL_LMA;
        . += 0x1000;

        __kernel_pd = . - KERNEL_LMA;
        . += 4 * 512 * 8;

        __kernel_end = . - KERNEL_VMA;
    }
}

