section .text
global _port_read_8, _port_write_8
global _port_read_32, _port_write_32

_port_read_8:
    mov dx, di
    in al, dx
    movzx eax, al
    ret

_port_write_8:
    mov dx, di
    mov al, sil 
    out dx, al
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
