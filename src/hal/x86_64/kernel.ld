ENTRY(_boot_kernel_gateway)

/* KERNEL_VMA = 0xFFFF800000000000; */
KERNEL_VMA = 0xFFFF800000000000;
KERNEL_LMA = 0x200000;

SECTIONS
{
    . = KERNEL_LMA;

    .boot ALIGN(0x1000) :
    {
        /* . = ALIGN(8); */
        KEEP(*(.boot));
    }

    . = KERNEL_VMA + KERNEL_LMA + 0x1000;

    .text ALIGN(8) : AT(ADDR(.text) - KERNEL_VMA)
    {
        __kernel_start = . - KERNEL_VMA;
        *(.text);
        *(.text.*);
    }

    .rodata ALIGN(8) : AT(ADDR(.rodata) - KERNEL_VMA)
    {
        *(.rodata);
        *(.rodata.*);
    }

    .data ALIGN(8) : AT(ADDR(.data) - KERNEL_VMA)
    {
        *(.data);
        *(.data.*);
    }

    .bss ALIGN(8) : AT(ADDR(.bss) - KERNEL_VMA)
    {
        __bss_start = . - KERNEL_VMA;
        *(COMMON);
        *(.bss);
        *(.bss.*);
        __bss_end = . - KERNEL_VMA;

        . = ALIGN(0x1000);
        __kernel_pml4 = . - KERNEL_VMA;
        . += 0x1000;

        . = ALIGN(0x1000);
        __kernel_pdpt = . - KERNEL_VMA;
        . += 0x1000;

        . = ALIGN(0x1000);
        __kernel_pd = . - KERNEL_VMA;
        . += 4 * 512 * 8;

        . = ALIGN(0x1000);
        __boot_stack_start = . - KERNEL_VMA;
        . += 0x1000 * 16;
        __boot_stack_end = . - KERNEL_VMA; 

        . = ALIGN(0x1000);
        __kernel_end = . - KERNEL_VMA;
    }

    /DISCARD/ :
    {
        *(*.eh_frame);
    }
}

