OUTPUT_FORMAT("elf64-x86-64")
ENTRY(_start)

SECTIONS
{
    . = 0x1000;

    .text ALIGN(0x1000) :
    {
        KEEP(*(.text.entry))
        *(.text)
        *(.text.*)
    }

    .rodata ALIGN(0x1000) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(0x1000) :
    {
        *(.data)
        *(.data.*)
    }

    .bss ALIGN(0x1000) (NOLOAD) :
    {
        __init_info_start = .;
        . += 0x1000;
        __init_info_end = .;

        __init_ipc_buffer_start = .;
        . += 0x1000;
        __init_ipc_buffer_end = .;

        __init_stack_start = .;
        . += 0x10000;
        __init_stack_end = .;

        *(.bss)
        *(.bss.*)
    }

    /DISCARD/ :
    {
        *(.eh_frame)
        *(.comment)
    }
}

