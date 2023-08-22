; INTERRUPT CONFIGURATION
global _load_idt, _enable_interrupt_all, _disable_interrupt_all

idtr DW 0
    DQ 0

_load_idt:
    mov [idtr], di
    mov [idtr + 2], rsi
    lidt [idtr]
    call _enable_interrupt_all
    ret

_enable_interrupt_all:
    sti
    ret

_disable_interrupt_all:
    cli
    ret

