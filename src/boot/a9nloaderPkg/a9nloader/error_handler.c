#include <Library/UefiLib.h>
#include <Uefi.h>

#include "error_handler.h"

EFI_STATUS handle_error(EFI_STATUS efi_status)
{
    if (EFI_ERROR(efi_status))
    {
        Print(L"EFI_ERROR: 0x%08x\r\n", efi_status);
        return efi_status;
    }
    Print(L"EFI_SUCCESS");
    return EFI_SUCCESS;
}