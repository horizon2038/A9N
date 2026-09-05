ENTRY(_boot_kernel_gateway)

KERNEL_VMA_BASE = 0xFFFF800000000000;
KERNEL_LMA_BASE = 0x200000;
BOOT_SIZE = 0x2000;
CPU_COUNT_MAX = 64;
KERNEL_DIRECT_MAP_GIB = 16;

MEMORY
{
    trampoline (rwx) : ORIGIN = 0x6000, LENGTH = 0x2000
    boot       (rwx) : ORIGIN = KERNEL_LMA_BASE, LENGTH = 1M

    /* kernel section */
    kernel_lma (rwx) : ORIGIN = KERNEL_LMA_BASE + BOOT_SIZE, LENGTH = 256M
    kernel_vma (rwx) : ORIGIN = KERNEL_VMA_BASE + KERNEL_LMA_BASE + BOOT_SIZE, LENGTH = 256M
}

SECTIONS
{
    . = 0x5000;
    __boot_ap_trampoline_start = .;
    . += 0x1000; /* reserve 1 page */
    __boot_ap_trampoline_end = .;
    __boot_ap_trampoline_gdtr = .;
    . += 0x1000;

    .boot : ALIGN(0x1000)
    {
        . = ALIGN(0x1000);
        /* . = ALIGN(8); */
        KEEP(*(.boot));
    } > boot

    .text : ALIGN(0x1000) 
    {
        . = ALIGN(0x1000);
        __kernel_start = . - KERNEL_VMA_BASE;
        *(.text);
        *(.text.*);
        *(.*text);
        *(.*text.*);
    } > kernel_vma AT > kernel_lma

    .rodata : ALIGN(0x1000)
    {
        . = ALIGN(0x1000);
        *(.rodata);
        *(.rodata.*);
        *(.*rodata);
        *(.*rodata.*);
    } > kernel_vma AT > kernel_lma

    .data : ALIGN(0x1000) 
    {
        . = ALIGN(0x1000);
        *(.data);
        *(.data.*);
        *(.*data);
        *(.*data.*);
    } > kernel_vma AT > kernel_lma

    .init_array : ALIGN(0x1000)
    {
        . = ALIGN(0x1000);
        __init_constructors_start = .;
        KEEP(*(
            .init_array
            .init_array.*
            .ctors
            .ctors.*
        ))
        __init_constructors_end = .;
    } > kernel_vma AT > kernel_lma

    .got : ALIGN(0x1000)
    {
        *(*.got);
        *(*.got.*);
    } > kernel_vma AT > kernel_lma

    .bss (NOLOAD) : ALIGN(0x1000) 
    {
        . = ALIGN(0x1000);
        __bss_start = . - KERNEL_VMA_BASE;
        *(COMMON);
        *(.bss);
        *(.bss.*);
        *(.*bss);
        *(.*bss.*);
        __bss_end = . - KERNEL_VMA_BASE;

        . = ALIGN(0x1000);
        __kernel_pml4 = . - KERNEL_VMA_BASE;
        . += 0x1000;

        . = ALIGN(0x1000);
        __kernel_pdpt = . - KERNEL_VMA_BASE;
        . += 0x1000;

        . = ALIGN(0x1000);
        __kernel_pd = . - KERNEL_VMA_BASE;
        . += KERNEL_DIRECT_MAP_GIB * 512 * 8;

        . = ALIGN(0x1000);
        __boot_stack_start = . - KERNEL_VMA_BASE;
        /*
         * The AP trampoline selects its temporary stack with the 8-bit Local APIC ID, not the
         * logical core number. Reserve every supported APIC-ID slot and one additional BSP-only
         * slot. In particular, the BSP must not share APIC-ID slot 63 when all 64 logical cores
         * are enabled.
         */
        . += 0x1000 * (CPU_COUNT_MAX + 1);
        __boot_stack_end = . - KERNEL_VMA_BASE; 

        . = ALIGN(0x1000);
        __kernel_end = . - KERNEL_VMA_BASE;
    } > kernel_vma AT > kernel_lma

    /DISCARD/ :
    {
        *(*.eh_frame);
    } > kernel_vma AT > kernel_lma
}
