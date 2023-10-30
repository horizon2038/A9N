ENTRY(_start)

SECTIONS
{
    .text :
    {
        *(.text);
        *(.text.*)
    }

    .rodata :
    {
        *(.rodata);
        *(.rodata.*);
    }

    .data :
    {
        *(.data);
        *(.data.*);
    }

    .bss :
    {
        PROVIDE(__bss_start = .);
        *(.bss);
        *(.bss.*);
        PROVIDE(__bss_end = .);
    }

    /DISCARD/ :
    {
        *(*.eh_frame);
    }
}
