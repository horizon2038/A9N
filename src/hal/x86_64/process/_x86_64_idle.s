global _idle

_idle:
    ; wait interrupt
    ; sti
    ; hlt
    jmp _idle
