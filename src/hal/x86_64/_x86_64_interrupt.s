extern x86_64_do_irq

%macro INTERRUPT_HANDLER 1

global interrupt_handler_%1
interrupt_handler_%1:
    push dword 0
    push dword %1
    jmp interrupt_handler_common

%endmacro

global interrupt_handlers

interrupt_handlers:
    i equ 0
%assign i 0
%rep 255
    %if i == 8 || (10<= i && i <= 14) || i == 17
    ; Exception { 8, 10, 11, 12, 13, 14, 17 }: with error code.
    ; cf. wiki.osdev.org/Exceptions
        align 16
        cli
        push i
        jmp interrupt_handler_common
        align 16
    %else
        align 16
        cli
        push 0 ; dummy error code.
        push i
        jmp interrupt_handler_common
        align 16 
    %endif
    %assign i i+1
%endrep

interrupt_handler_common:
    xchg rdi, [rsp] ; push i (in stack-top) => rdi (1st argument)
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

    mov rsi, rsp

    ; call x86_64_do_irq in C with interrupt number as argument.
    call x86_64_do_irq

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
    
    add rsp, 8

    ; set current rsp to tss.rsp0

    iretq
    
