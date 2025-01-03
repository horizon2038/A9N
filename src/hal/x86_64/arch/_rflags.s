section .text

global _read_rflags:
    pushfq
    pop rax

    ret
