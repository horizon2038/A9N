ENTRY(_boot_kernel_gateway)

KERNEL_VMA_BASE = 0xFFFF800000000000;
KERNEL_LMA_BASE = 0x00080000;
BOOT_SIZE = 0x20000;
CPU_COUNT_MAX = 4;
KERNEL_DIRECT_MAP_GIB = 16;
__kernel_image_start = KERNEL_LMA_BASE;
__kernel_image_start_virtual = KERNEL_VMA_BASE + KERNEL_LMA_BASE;

MEMORY
{
    boot       (rwx) : ORIGIN = KERNEL_LMA_BASE, LENGTH = BOOT_SIZE
    kernel_lma (rwx) : ORIGIN = KERNEL_LMA_BASE + BOOT_SIZE, LENGTH = 256M
    kernel_vma (rwx) : ORIGIN = KERNEL_VMA_BASE + KERNEL_LMA_BASE + BOOT_SIZE, LENGTH = 256M
}

SECTIONS
{
    .boot : ALIGN(0x1000)
    {
        KEEP(*(.boot.entry))
        *(.boot .boot.*)
    } > boot

    .text : ALIGN(0x1000)
    {
        __kernel_start = . - KERNEL_VMA_BASE;
        *(.text .text.*)
    } > kernel_vma AT > kernel_lma

    .rodata : ALIGN(0x1000)
    {
        *(.rodata .rodata.*)
    } > kernel_vma AT > kernel_lma

    .data : ALIGN(0x1000)
    {
        *(.data .data.*)
    } > kernel_vma AT > kernel_lma

    .init_array : ALIGN(0x1000)
    {
        __init_constructors_start = .;
        KEEP(*(.init_array .init_array.* .ctors .ctors.*))
        __init_constructors_end = .;
    } > kernel_vma AT > kernel_lma

    .bss (NOLOAD) : ALIGN(0x1000)
    {
        __bss_start = . - KERNEL_VMA_BASE;
        *(COMMON)
        *(.bss .bss.*)
        __bss_end = . - KERNEL_VMA_BASE;

        . = ALIGN(0x1000);
        __kernel_l0 = . - KERNEL_VMA_BASE;
        . += 0x1000;
        __kernel_l1 = . - KERNEL_VMA_BASE;
        . += 0x1000;
        __kernel_l2 = . - KERNEL_VMA_BASE;
        . += KERNEL_DIRECT_MAP_GIB * 0x1000;

        . = ALIGN(0x1000);
        __boot_stack_start = . - KERNEL_VMA_BASE;
        . += 0x2000 * CPU_COUNT_MAX;
        __boot_stack_end = . - KERNEL_VMA_BASE;

        . = ALIGN(0x1000);
        __kernel_end = . - KERNEL_VMA_BASE;
        __kernel_end_virtual = .;
    } > kernel_vma AT > kernel_lma

    /DISCARD/ :
    {
        *(.comment .note.* .eh_frame .eh_frame_hdr)
    }
}
