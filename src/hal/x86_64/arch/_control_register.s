section .text

global _write_cr0
global _read_cr0

global _read_cr2

global _write_cr3
global _read_cr3

global _write_cr4
global _read_cr4

_write_cr0:
    mov cr0, rdi
    ret

_read_cr0:
    mov rax, cr0
    ret

; when a page fault is triggered, the target address is stored.
_read_cr2:
    mov rax, cr2
    ret

; root page table
_write_cr3:
    mov cr3, rdi
    ret

_read_cr3:
    mov rax, cr3
    ret

_write_cr4:
    mov cr4, rdi
    ret

_read_cr4:
    mov rax, cr4
    ret
