section .text

global _vmxon
global _vmclear
global _vmptrld
global _vmread
global _vmwrite
global _vmlaunch

; void _vmxon(a9n::physical_address *vmxon_region)
_vmxon:
    vmxon [rdi]

    ret

; void _vmclear(a9n::physical_address *vmcs_region)
_vmclear:
    vmclear [rdi]

    ret

; void _vmptrld(a9n::physical_address *vmcs_region)
_vmptrld:
    vmptrld [rdi]

    ret

; uint64_t _vmread(vmcs_field field)
_vmread:
    vmread rax, rdi

    ret

; void _vmwrite(vmcs_field field, uint64_t value)
_vmwrite:
    vmwrite rdi, rsi

    ret

_vmlaunch:
    vmlaunch

    ret
