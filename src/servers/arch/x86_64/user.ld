OUTPUT_FORMAT("elf64-x86-64")

ENTRY(_start)

SECTIONS
{
    . = 0x1000;

    .init_info ALIGN(0x1000) :
    {
    }

    .text ALIGN(0x1000) :
    {
        *(.text);
        *(.text.*)
    }

    .rodata ALIGN(0x1000) :
    {
        *(.rodata);
        *(.rodata.*);
    }

    .data ALIGN(0x1000) :
    {
        *(.data);
        *(.data.*);
    }

    .bss ALIGN(0x1000) :
    {
        __init_info_start = .;
        /* allocate 4KiB */
        . += 0x1000;
        __init_info_end = .;

        __init_ipc_buffer_start = .;
        /* allocate 4KiB */
        . += 0x1000;
        __init_ipc_buffer_end = .;

        __init_stack_start = .;
        /* allocate 4KiB */
        . += 0x1000;
        __init_stack_end = .;

        __bss_start = .;
        *(.bss);
        *(.bss.*);
        __bss_end = .;
    }

    /DISCARD/ :
    {
        *(*.eh_frame);
    }
}
