section .boot
default rel
BITS 64

global trampoline
extern kernel_entry

%define KERNEL_VIRTUAL_BASE 0xffff800000000000

align 4096
; page_directory:
;     resq 512
; page_table:
;     resq 512*512
extern __kernel_pml4
extern __kernel_pdpt
extern __kernel_pd

extern __kernel_start
extern __kernel_end

trampoline:
    ; Calculate the size from __kernel_start to __kernel_end
    lea rax, __kernel_end
    mov rbx, __kernel_start
    sub rax, rbx
    ; sub rax, __kernel_start

    push rdi

    ; Convert the size to pages
    add rax, 0xFFF     ; Add 4095 to round up
    shr rax, 12        ; Divide by 4096 to get the page count
    ; Prepare arguments for map_pages
    lea rdi, [__kernel_start]  ; phys_start
    mov rsi, KERNEL_VIRTUAL_BASE ; virt_start
    add rsi, rdi
    mov rdx, rax  ; page_count
    ; Call map_pages
    call map_pages
    ; Jump to the kernel entry point
    ; RDI already contains the address of boot_info

    ; Flush TLB
    mov rax, [__kernel_pml4]
    mov cr3, rax

    pop rdi
    jmp kernel_entry

    ret

map_pages:
    ; Arguments: rdi = phys_start, rsi = virt_start, rdx = page_count

.loop:
    test rdx, rdx
    jz .done

    ; Calculate PML4, PDPT, PDE, and PTE indices
    mov rcx, rsi  ; Copy virt_start to rcx
    shr rcx, 39   ; Get PML4 index
    and rcx, 0x1FF

    mov r8, rsi   ; Copy virt_start to r8
    shr r8, 30    ; Get PDPT index
    and r8, 0x1FF

    mov r9, rsi   ; Copy virt_start to r9
    shr r9, 21    ; Get PDE index
    and r9, 0x1FF

    mov rax, rsi  ; Copy virt_start to rax
    shr rax, 12   ; Get PTE index
    and rax, 0x1FF

    ; Set up PML4 entry
    mov rbx, [__kernel_pml4]
    add rbx, rcx
    shl rbx, 3    ; Multiply by 8 since each entry is 8 bytes
    mov r10, [__kernel_pdpt]
    or r10, 0x003  ; PDPT base address with Present and writable flags
    mov [rbx], r10

    ; Set up PDPT entry
    mov rbx, [__kernel_pdpt]
    add rbx, r8
    shl rbx, 3    ; Multiply by 8 since each entry is 8 bytes
    mov r10, [__kernel_pd]
    or r10, 0x003  ; PD base address with Present and writable flags
    mov [rbx], r10

    ; Set up PDE
    mov rbx, [__kernel_pd]
    add rbx, r9
    shl rbx, 3    ; Multiply by 8 since each entry is 8 bytes
    mov r10, [__kernel_pd]
    add r10, 0x1000  ; Assuming the page table starts at __kernel_pd + 0x1000
    or r10, 0x003  ; Page table base address with Present and writable flags
    mov [rbx], r10

    ; Set up PTE
    mov rbx, [__kernel_pd]
    add rbx, 0x1000
    add rbx, rax
    shl rbx, 3    ; Multiply by 8 since each entry is 8 bytes
    mov rax, rdi  ; Physical address
    or rax, 0x3   ; Present and writable
    mov [rbx], rax

    ; Move to the next page
    add rdi, 0x1000  ; Increment phys_start by PAGE_SIZE
    add rsi, 0x1000  ; Increment virt_start by PAGE_SIZE
    dec rdx  ; Decrement page_count
    jmp .loop

.done:
    ret

