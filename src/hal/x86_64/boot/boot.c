#include <stdint.h>

#define KERNEL_VMA 0xFFFF800000000000;
#define KERNEL_LMA 0x200000;

extern uint64_t __kernel_start, __kernel_end;
extern uint64_t __kernel_pml4, __kernel_pdpt, __kernel_pd;
extern uint8_t  __boot_stack_end;

extern void __attribute__((section(".text"))) kernel_entry(void *boot_info);

void __attribute__((section(".boot"))) init_page_table(void)
{
    uint64_t *pml4_table = (uint64_t *)&__kernel_pml4;
    // Temporarily, PML4[0] is pointing to the same area as PML[256], but this
    // is temporary, and it is necessary to reconfigure this within the kernel
    // and unmap the identity map.
    pml4_table[0]   = (uint64_t)&__kernel_pdpt | 0x3;
    pml4_table[256] = (uint64_t)&__kernel_pdpt | 0x3;

    // Set up the PDPT table
    uint64_t *pdpt_table = (uint64_t *)&__kernel_pdpt;
    for (uint64_t i = 0; i < 4; i++)
    {
        pdpt_table[i] = (uint64_t)(&__kernel_pd + i * 512) | 0x3;
    }

    // Set up the PD table for 2MiB pages
    // TODO: reconsider virtual-address-map-policy
    uint64_t *pd_table = (uint64_t *)&__kernel_pd;
    for (uint64_t i = 0; i < 4 * 512; i++)
    {
        // 512 entries for 2MiB pages cover 1GiB
        uint64_t physical_addr = i * 0x200000; // 2MiB per page
        uint64_t virtual_addr  = 0xFFFF800000000000 + i * 0x200000;
        pd_table[i]
            = physical_addr | 0x83; // Present, Writeable, and Page Size flag
    }
}
