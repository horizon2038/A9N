global _idle

_idle:
    ; wait interrupt
    sti
    hlt
    cli
    jmp _idle
