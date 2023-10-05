global _load_cr3 ; void load_cr3(uint64_t address);
global _flush_tlb ; void flash_tlb();

_load_cr3:
    mov cr3. rdi
    ret

_flush_tlb:
    mov eax, cr3
    mov cr3, eax

