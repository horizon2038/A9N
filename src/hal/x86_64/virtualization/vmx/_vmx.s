section .text

global _vmxon
global _vmclear
global _vmptrld

; void _vmxon(a9n::physical_address vmxon_region)
_vmxon:
    vmxon rdi

    ret

; void _vmclear(a9n::physical_address vmcs_region)
_vmclear:
    vmclear rdi

    ret

; void _vmptrld(a9n::physical_address vmcs_region)
_vmptrld:
    vmptrld rdi

    ret

