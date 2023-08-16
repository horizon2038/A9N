section .text
global _read, _write

_read:
    mov rax, rdi
    in al, dx
;    mov dx, rdi
;    in al, dx
;    movzx rax, al
;    ret

_write:
    mov rdx, rdi
    mov rax, rsi
    out dx, ax
    ret
;    mov dx, rdi
;    mov al, sil
;    out dx, al
;    ret
