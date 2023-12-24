section .text
global _read, _write

_read:
    mov rdx, rdi
    in ax, dx
    nop
    ret

_write:
    mov rdx, rdi
    mov rax, rsi
    out dx, ax
    nop
    ret
