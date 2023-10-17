ENTRY(kernel_entry)

PHYSICAL_BASE = 0x100000;
VIRTUAL_BASE = 0xFFFFFFFF80000000 + PHYSICAL_BASE;

SECTIONS
{
    . = VIRTUAL_BASE;

    .text : AT(PHYSICAL_BASE)
    {
        _code = .;
        *(.text)
        *(.rodata*)
        . = ALIGN(4096);
    }

   .data : AT(PHYSICAL_BASE + (ADDR(.text) - VIRTUAL_BASE) + SIZEOF(.text))
   {
        _data = .;
        *(.data)
        . = ALIGN(4096);
   }

   .eh_frame : AT(PHYSICAL_BASE + (ADDR(.data) - VIRTUAL_BASE) + SIZEOF(.data))
   {
       _ehframe = .;
       *(.eh_frame)
        . = ALIGN(4096);
   }

   .ctors : AT(PHYSICAL_BASE + (ADDR(.eh_frame) - VIRTUAL_BASE) + SIZEOF(.eh_frame))
   {
        _ctors_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP(*(.ctors))
        . = ALIGN(8);
        _ctors_end = .;
    }

   .dtors : AT(PHYSICAL_BASE + (ADDR(.ctors) - VIRTUAL_BASE) + SIZEOF(.ctors))
   {
        _dtors_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP(*(.dtors))
        . = ALIGN(8);
        _dtors_end = .;
    }

   .bss : AT(PHYSICAL_BASE + (ADDR(.dtors) - VIRTUAL_BASE) + SIZEOF(.dtors))
   {
       _bss = .;
       *(.bss)
       *(COMMON)
       . = ALIGN(4096);
   }

   _end = .;

   /DISCARD/ :
   {
        *(.comment)
   }
}

