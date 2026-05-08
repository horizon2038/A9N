global _load_cr3, _flush_tlb, _invalidate_page

_load_cr3: ; void load_cr3(uint64_t address()
    mov cr3, rdi
    ret

_flush_tlb: ; void flash_tlb()
    mov rax, cr3
    mov cr3, rax
    ret

_invalidate_page:
    invlpg [rsi]
    ret
