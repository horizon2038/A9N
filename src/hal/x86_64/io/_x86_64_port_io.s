section .text
global _port_read_8, _port_write_8
global _port_read_32, _port_write_32

_port_read_8:
    mov rdx, rdi
    in ax, dx
    nop
    ret

_port_write_8:
    mov rdx, rdi
    mov rax, rsi
    out dx, ax
    nop
    ret

_port_read_32:
    mov rdx, rdi
    in eax, dx
    ret

_port_write_32:
    mov rdx, rdi
    mov rax, rsi
    out dx, eax
    ret
