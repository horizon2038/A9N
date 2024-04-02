#include "acpi.h"

#include <Library/BaseMemoryLib.h>
#include <stdint.h>

void *find_rsdp(EFI_SYSTEM_TABLE *system_table)
{
    for (uintmax_t i = 0; i < system_table->NumberOfTableEntries; i++)
    {
        if (!CompareGuid(
                &gEfiAcpiTableGuid,
                &system_table->ConfigurationTable[i].VendorGuid
            ))
        {
            continue;
        }
        return (void *)(system_table->ConfigurationTable[i].VendorTable);
    }
    return NULL;
}
