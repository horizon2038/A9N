section .text

global _switch_context

; void _switch_context(uint64_t *preview_stack_pointer, uint64_t *next_stack_pointer)
_switch_context:
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15
    pushfq

    mov [rdi], rsp ; sync address between in-process stack pointer and rsp
    mov rsp, [rsi] ; change stack (pointer)

    popfq
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp

    ret

