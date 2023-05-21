#include <Uefi.h>
#include <Library/UefiLib.h>

#include "error_handler.h"

EFI_STATUS handle_error(EFI_STATUS efi_status)
{
    if(EFI_ERROR(efi_status))
    {
        Print(L"EFI_ERROR\r\n");
        return efi_status;
    }
    return EFI_SUCCESS;
}