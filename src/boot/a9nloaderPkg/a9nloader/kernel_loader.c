#include "kernel_loader.h"

#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Uefi.h>

#include "elf_info_logger.h"
#include "file_reader.h"

static EFI_STATUS read_elf_header(EFI_FILE_PROTOCOL *file, elf64_header **header);

static EFI_STATUS read_elf_program_header_table(
    EFI_FILE_PROTOCOL     *file,
    elf64_header          *header,
    elf64_program_header **program_header
);
static EFI_STATUS
    allocate_elf_memory_direct(elf64_header *header, elf64_program_header *program_header);

static EFI_STATUS allocate_elf_memory_any_address(
    elf64_header         *header,                   // input
    elf64_program_header *program_header_table,     // input
    uint64_t             *physical_address_offset,  // output
    uint64_t             *elf_segment_physical_size // output
);

static void calculate_elf_segment_range(
    elf64_header         *header,                // input
    elf64_program_header *program_header_table,  // input
    uint64_t             *start_segment_address, // output
    uint64_t             *end_segment_address    // output
);

static EFI_STATUS load_elf_segment(
    EFI_FILE_PROTOCOL    *file,
    elf64_header         *header,
    elf64_program_header *program_header,
    uint64_t              physical_address_offset
);

static void locate_elf_segment(
    elf64_header         *header,
    elf64_program_header *program_header,
    void                 *buffer,
    uint64_t              physical_address_offset
);

static void zero_clear(elf64_program_header *program_header, uint64_t physical_address_offset);

EFI_STATUS load_kernel(EFI_FILE_PROTOCOL *kernel, uint64_t *entry_point)
{
    EFI_STATUS            efi_status = EFI_SUCCESS;
    elf64_header         *loaded_elf64_header;
    elf64_program_header *loaded_elf64_program_header_table;

    efi_status = read_elf_header(kernel, &loaded_elf64_header);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    efi_status
        = read_elf_program_header_table(kernel, loaded_elf64_header, &loaded_elf64_program_header_table);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    efi_status = allocate_elf_memory_direct(loaded_elf64_header, loaded_elf64_program_header_table);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    // offset 0 (direct)
    efi_status = load_elf_segment(kernel, loaded_elf64_header, loaded_elf64_program_header_table, 0);
    if (EFI_ERROR(efi_status))
    {
        return efi_status;
    }

    *entry_point = loaded_elf64_header->entry_point_address;

    // free unused data
    FreePool(loaded_elf64_header);
    FreePool(loaded_elf64_program_header_table);

    return efi_status;
}

EFI_STATUS load_init_server(
    EFI_FILE_PROTOCOL *init_server_file,
    uint64_t          *init_server_loaded_address,
    uint64_t          *init_server_memory_size,
    uint64_t          *init_server_entry_point,
    uint64_t          *init_server_info_address,
    uint64_t          *init_server_ipc_buffer_address
)
{
    EFI_STATUS efi_status = EFI_SUCCESS;

    elf64_header         *loaded_elf64_header;
    elf64_program_header *loaded_elf64_program_header_table;

    efi_status = read_elf_header(init_server_file, &loaded_elf64_header);
    if (EFI_ERROR(efi_status))
    {
        Print(L"failed to read_elf_header\r\n");
        return efi_status;
    }

    efi_status = read_elf_program_header_table(
        init_server_file,
        loaded_elf64_header,
        &loaded_elf64_program_header_table
    );
    if (EFI_ERROR(efi_status))
    {
        Print(L"failed to read_elf_program_header_table\r\n");
        return efi_status;
    }

    efi_status = allocate_elf_memory_any_address(
        loaded_elf64_header,
        loaded_elf64_program_header_table,
        init_server_loaded_address,
        init_server_memory_size
    );
    if (EFI_ERROR(efi_status))
    {
        Print(L"failed to allocate_elf_memory_any_address\r\n");
        return efi_status;
    }

    efi_status = load_elf_segment(
        init_server_file,
        loaded_elf64_header,
        loaded_elf64_program_header_table,
        *init_server_loaded_address
    );
    if (EFI_ERROR(efi_status))
    {
        Print(L"failed to load_elf_segment\r\n");
        return efi_status;
    }

    *init_server_entry_point        = loaded_elf64_header->entry_point_address;
    *init_server_info_address       = 0;
    *init_server_ipc_buffer_address = 0;

    // free unused data
    FreePool(loaded_elf64_header);
    FreePool(loaded_elf64_program_header_table);

    return efi_status;
}

static EFI_STATUS read_elf_header(EFI_FILE_PROTOCOL *kernel, elf64_header **header)
{
    EFI_STATUS efi_status = EFI_SUCCESS;

    efi_status            = read_file(kernel, 0, sizeof(elf64_header), (void **)header);
    print_elf_header_info(*header);

    return efi_status;
}

static EFI_STATUS read_elf_program_header_table(
    EFI_FILE_PROTOCOL     *kernel,
    elf64_header          *header,
    elf64_program_header **program_header
)
{
    EFI_STATUS efi_status = EFI_SUCCESS;

    uint64_t program_header_table_size = sizeof(elf64_program_header) * header->program_header_number;
    // Print(L"");
    efi_status = read_file(
        kernel,
        header->program_header_offset,
        program_header_table_size,
        (void **)program_header
    );

    return efi_status;
}

static EFI_STATUS
    allocate_elf_memory_direct(elf64_header *header, elf64_program_header *program_header_table)
{
    EFI_STATUS efi_status            = EFI_SUCCESS;
    uint64_t   start_segment_address = 0;
    uint64_t   end_segment_address   = 0;
    uint64_t   segment_size          = 0;

    calculate_elf_segment_range(header, program_header_table, &start_segment_address, &end_segment_address);
    segment_size = EFI_SIZE_TO_PAGES(end_segment_address - start_segment_address);
    efi_status
        = gBS->AllocatePages(AllocateAddress, EfiLoaderData, segment_size, &start_segment_address);

    return efi_status;
}

static EFI_STATUS allocate_elf_memory_any_address(
    elf64_header         *header,
    elf64_program_header *program_header_table,
    uint64_t             *physical_address_offset,
    uint64_t             *elf_segment_physical_size
)
{
    EFI_STATUS efi_status            = EFI_SUCCESS;
    uint64_t   start_segment_address = 0;
    uint64_t   end_segment_address   = 0;
    uint64_t   segment_size          = 0;

    calculate_elf_segment_range(header, program_header_table, &start_segment_address, &end_segment_address);
    segment_size = EFI_SIZE_TO_PAGES(end_segment_address - start_segment_address);
    efi_status
        = gBS->AllocatePages(AllocateAnyPages, EfiLoaderData, segment_size, &start_segment_address);

    *physical_address_offset = start_segment_address;

    return efi_status;
}

static void calculate_elf_segment_range(
    elf64_header         *header,
    elf64_program_header *program_header_table,
    uint64_t             *start_segment_address,
    uint64_t             *end_segment_address
)
{
    elf64_program_header *program_header;

    *start_segment_address = MAX_UINT64;
    *end_segment_address   = 0;

    for (uint16_t i = 0; i < header->program_header_number; ++i)
    {
        program_header = &(program_header_table[i]);
        if (program_header->type != PT_LOAD)
        {
            continue;
        }
        *start_segment_address = MIN(*start_segment_address, program_header->physical_address);
        *end_segment_address
            = MAX(*end_segment_address, program_header->physical_address + program_header->memory_size);
    }
}

static EFI_STATUS load_elf_segment(
    EFI_FILE_PROTOCOL    *kernel,
    elf64_header         *header,
    elf64_program_header *program_header_table,
    uint64_t              physical_address_offset
)
{
    EFI_STATUS            efi_status = EFI_SUCCESS;
    elf64_program_header *program_header;
    void                 *buffer;

    for (uint16_t i = 0; i < header->program_header_number; ++i)
    {
        program_header = &(program_header_table[i]);
        if (program_header->type != PT_LOAD)
        {
            continue;
        }
        print_elf_program_header_info(program_header);
        efi_status = read_file(kernel, program_header->offset, program_header->file_size, &buffer);
        if (EFI_ERROR(efi_status))
            return efi_status;
        locate_elf_segment(header, program_header, buffer, physical_address_offset);
        zero_clear(program_header, physical_address_offset);
        FreePool(buffer);
    }

    return efi_status;
}

static void locate_elf_segment(
    elf64_header         *header,
    elf64_program_header *program_header,
    void                 *source_buffer,
    uint64_t              physical_address_offset
)
{
    CopyMem(
        (void *)(program_header->physical_address + physical_address_offset),
        source_buffer,
        program_header->file_size
    );
}

static void zero_clear(elf64_program_header *program_header, uint64_t physical_address_offset)
{
    if (program_header->file_size < program_header->memory_size)
    {
        gBS->SetMem(
            (void *)(program_header->physical_address + physical_address_offset
                     + program_header->file_size),
            program_header->memory_size - program_header->file_size,
            0
        );
    }
}
