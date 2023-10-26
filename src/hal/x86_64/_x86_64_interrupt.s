%macro INTERRUPT_HANDLER 1

global interrupt_handler_%1
interrupt_handler_%1:
    push dword 0
    push dword %1
    jmp interrupt_handler_common

%endmacro

; %assign intnum 0
; %rep 8 - intnum
    

interrupt_handler_common:
    xchg rdi, [rsp]
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    extern do_irq
    ; call do_irq in C with interrupt number as argument.
    ; call do_irq

    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    pop rdi
    
