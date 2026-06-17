#include "liba9n/libc/string.hpp"
#include <kernel/boot/init.hpp>

#include <kernel/capability/address_space.hpp>
#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_node.hpp>
#include <kernel/capability/frame_capability.hpp>
#include <kernel/capability/generic.hpp>
#include <kernel/capability/interrupt_region.hpp>
#include <kernel/capability/io_port_capability.hpp>
#include <kernel/capability/page_table_capability.hpp>
#include <kernel/capability/process_control_block.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/memory/memory_type.hpp>

#include <kernel/ipc/ipc_buffer.hpp>
#include <kernel/memory/memory.hpp>
#include <kernel/process/process.hpp>
#include <kernel/process/process_manager.hpp>
#include <kernel/types.hpp>

#include <kernel/process/scheduler.hpp>
#include <kernel/utility/logger.hpp>
#include <kernel/version.hpp>

#include <hal/interface/memory_manager.hpp>
#include <hal/interface/process_manager.hpp>

#include <liba9n/common/allocator.hpp>
#include <liba9n/common/calculate.hpp>
#include <liba9n/common/enum.hpp>
#include <liba9n/common/not_null.hpp>

namespace a9n::kernel
{
    using page_size_memory = liba9n::std::array<uint8_t, a9n::PAGE_SIZE>;

    // assign only once; no memory freed
    liba9n::linear_allocator<a9n::PAGE_SIZE * 512> init_allocator {};

    // forward declaration
    template<typename T>
    struct capability_instance
    {
        capability_slot &slot;
        T               &component;
    };

    // clang-format off
    kernel_result try_make_init(const boot_info &info);
    liba9n::result<liba9n::not_null<init_info>, kernel_error> try_make_init_info(const boot_info &info);
    liba9n::result<liba9n::not_null<init_info>, kernel_error> try_create_init_info(const boot_info &info);
    liba9n::result<capability_instance<process_control_block>, kernel_error> try_create_init_process_control_block(const boot_info &info, init_info &init_info_page);
    liba9n::result<liba9n::not_null<capability_slot>, kernel_error> try_create_process_control_block_slot(void);
    liba9n::result<liba9n::not_null<process_control_block>, kernel_error> try_create_process_control_block(capability_slot &slot, const boot_info &info, init_info &init_info_page);
    kernel_result try_create_init_process_nodes(capability_instance<process_control_block> &pcb);
    liba9n::result<capability_instance<capability_node>, kernel_error> try_make_init_node(capability_instance<capability_node> &root_node, uintmax_t index, uintmax_t size_radix);
    liba9n::result<capability_instance<capability_node>, kernel_error> try_make_node(capability_slot &slot, a9n::word size_radix);
    kernel_result try_configure_init_info(init_info &info, const boot_info &boot);
    kernel_result try_configure_init_generic_descriptors(const memory_info &memory, init_info &init);
    liba9n::result<memory_map_entry, kernel_error> try_configure_generic_descriptor_from_memory_map(const memory_map_entry &entry, generic_descriptor &descriptor);
    kernel_result try_configure_init_process_control_block(capability_instance<process_control_block> &pcb, const init_info &info, const boot_info &boot);
    kernel_result try_configure_init_io_ports(process_control_block &pcb);
    kernel_result try_configure_init_address_space(process_control_block &pcb, const boot_info &boot, const init_info &init);
    kernel_result try_configure_init_root_address_space(process_control_block &pcb);
    kernel_result try_configure_init_page_tables(process_control_block &pcb, const init_image_info &info);
    kernel_result try_configure_init_frames(process_control_block &pcb, const init_image_info &info);
    kernel_result try_configure_init_generics(process_control_block &pcb, const init_info &info);
    kernel_result try_configure_init_interrupt_region(process_control_block &pcb);
    // clang-format on

    kernel_result create_init(const boot_info &info)
    {
        using a9n::kernel::utility::logger;
        logger::printk("Making init process ...\n");

        // 1. create (basically assign memory and create empty structures)
        auto init_info = TRY(try_create_init_info(info));
        auto init_pcb  = TRY(try_create_init_process_control_block(info, *init_info));

        // 2. configure (fill the information and make the structures work)
        TRY_VOID(try_configure_init_info(*init_info, info));
        TRY_VOID(try_configure_init_process_control_block(init_pcb, *init_info, info));

        logger::printk("Init process is created and configured successfully!\n");
        return {};
    }

    liba9n::result<liba9n::not_null<init_info>, kernel_error>
        try_create_init_info(const boot_info &info)
    {
        using a9n::kernel::utility::logger;
        logger::printk("Creating init information ...\n");

        a9n::physical_address init_info_address
            = info.boot_init_image_info.loaded_address + info.boot_init_image_info.init_info_address;
        if (!init_info_address)
        {
            return kernel_error::NO_SUCH_ADDRESS;
        }

        init_info &init_info_page
            = *a9n::kernel::physical_to_virtual_pointer<init_info>(init_info_address);

        return liba9n::not_null<init_info> { init_info_page };
    }

    liba9n::result<capability_instance<process_control_block>, kernel_error>
        try_create_init_process_control_block(const boot_info &info, init_info &init_info_page)
    {
        using a9n::kernel::utility::logger;
        logger::printk("Creating init process control block ...\n");

        // create init pcb on memory
        auto init_pcb_slot = TRY(try_create_process_control_block_slot());
        auto init_pcb = TRY(try_create_process_control_block(*init_pcb_slot, info, init_info_page));
        auto init_pcb_instance = capability_instance<process_control_block> {
            .slot      = *init_pcb_slot,
            .component = *init_pcb
        };
        TRY_VOID(try_create_init_process_nodes(init_pcb_instance));

        // configure process control block

        return capability_instance<process_control_block> {
            .slot      = *init_pcb_slot,
            .component = *init_pcb
        };
    }

    liba9n::result<liba9n::not_null<capability_slot>, kernel_error>
        try_create_process_control_block_slot(void)
    {
        using a9n::kernel::utility::logger;
        logger::printk("Creating init process control block slot ...\n");

        return init_allocator.allocate<capability_slot>(1).transform_error(
            [](liba9n::allocator_error e) -> kernel_error
            {
                return kernel_error::UNEXPECTED;
            }
        );
    }

    liba9n::result<liba9n::not_null<process_control_block>, kernel_error>
        try_create_process_control_block(capability_slot &slot, const boot_info &info, init_info &init_info_page)
    {
        using a9n::kernel::utility::logger;
        logger::printk("Creating init process control block on memory ...\n");

        return init_allocator.allocate<process_control_block>(1)
            .transform_error(
                [](liba9n::allocator_error e) -> kernel_error
                {
                    return kernel_error::UNEXPECTED;
                }
            )
            .and_then(
                [&](liba9n::not_null<process_control_block> pcb)
                    -> liba9n::result<liba9n::not_null<process_control_block>, kernel_error>
                {
                    return try_configure_process_control_block_slot(slot, *pcb)
                        .and_then(
                            [&](void) -> liba9n::result<liba9n::not_null<process_control_block>, kernel_error>
                            {
                                return pcb;
                            }
                        );
                }
            );
    }

    kernel_result try_create_init_process_nodes(capability_instance<process_control_block> &pcb)
    {
        using a9n::kernel::utility::logger;
        logger::printk("Creating init process nodes ...\n");

        logger::printk("Creating root node ...\n");
        [[maybe_unused]] auto root_node = TRY(try_make_node(
            pcb.component.process_core.root_slot,
            liba9n::calculate_radix_floor(INITIAL_PROCESS_ROOT_NODE_COUNT)
        ));

        logger::printk("Copying process control block reference to the slot ...\n");
        auto pcb_slot = TRY(
            root_node.component
                .retrieve_slot(liba9n::enum_cast(init_slot_offset::PROCESS_CONTROL_BLOCK))
                .transform_error(
                    [](capability_lookup_error e) -> kernel_error
                    {
                        return kernel_error::NO_SUCH_ADDRESS;
                    }
                )
        );
        TRY_VOID(try_copy_capability_slot(*pcb_slot, pcb.slot));

        logger::printk("Copying root node reference to the slot ...\n");
        auto root_node_slot = TRY(
            root_node.component
                .retrieve_slot(liba9n::enum_cast(init_slot_offset::PROCESS_ROOT_NODE))
                .transform_error(
                    [](capability_lookup_error e) -> kernel_error
                    {
                        return kernel_error::NO_SUCH_ADDRESS;
                    }
                )
        );
        TRY_VOID(try_copy_capability_slot(*root_node_slot, pcb.component.process_core.root_slot));

        logger::printk("Creating page table node ...\n");
        [[maybe_unused]] auto page_table_node = TRY(try_make_init_node(
            root_node,
            liba9n::enum_cast(init_slot_offset::PROCESS_PAGE_TABLE_NODE),
            liba9n::calculate_radix_floor(INITIAL_PAGE_TABLE_COUNT_MAX)
        ));

        logger::printk("Creating frame node ...\n");
        [[maybe_unused]] auto frame_node = TRY(try_make_init_node(
            root_node,
            liba9n::enum_cast(init_slot_offset::PROCESS_FRAME_NODE),
            liba9n::calculate_radix_floor(INITIAL_FRAME_COUNT_MAX)
        ));

        logger::printk("Creating generic node ...\n");
        [[maybe_unused]] auto generic_node = TRY(try_make_init_node(
            root_node,
            liba9n::enum_cast(init_slot_offset::GENERIC_NODE),
            liba9n::calculate_radix_floor(INITIAL_GENERIC_COUNT_MAX)
        ));

        return {};
    }

    liba9n::result<capability_instance<capability_node>, kernel_error> try_make_init_node(
        capability_instance<capability_node> &root_node,
        uintmax_t                             index,
        uintmax_t                             size_radix
    )
    {
        auto get_slot =
            [&](a9n::word index) -> liba9n::result<liba9n::not_null<capability_slot>, kernel_error>
        {
            return root_node.slot.component->retrieve_slot(index)
                .transform(
                    [](capability_slot *slot) -> liba9n::not_null<capability_slot>
                    {
                        return *slot;
                    }
                )
                .transform_error(
                    [&]([[maybe_unused]] capability_lookup_error e) -> kernel_error
                    {
                        return kernel_error::NO_SUCH_ADDRESS;
                    }
                );
        };

        auto target_slot = TRY(get_slot(index));

        return try_make_node(target_slot.get(), size_radix);
    }

    liba9n::result<capability_instance<capability_node>, kernel_error>
        try_make_node(capability_slot &slot, a9n::word size_radix)
    {
        // allocate slots
        auto slots_count  = static_cast<a9n::word>(1) << size_radix;
        auto slots_result = init_allocator.allocate<capability_slot>(slots_count);
        if (!slots_result)
        {
            return kernel_error::UNEXPECTED;
        }
        auto slots = slots_result.unwrap();

        // allocate node
        auto node_result
            = init_allocator.allocate<a9n::kernel::capability_node>(1, 0, size_radix, &slots.get());
        if (!node_result)
        {
            return kernel_error::UNEXPECTED;
        }
        auto node = node_result.unwrap();

        // configure node
        TRY_VOID(try_configure_capability_node_slot(slot, node.get()));

        return capability_instance<capability_node> { .slot = slot, .component = node.get() };
    }

    kernel_result try_configure_init_info(init_info &info, const boot_info &boot)
    {
        using a9n::kernel::utility::logger;
        logger::printk("Configuring init information ...\n");

        // configure version
        liba9n::semantic_version version { KERNEL_VERSION_STRING };
        info.kernel_major_version = version.current_major();
        info.kernel_minor_version = version.current_minor();
        info.kernel_patch_version = version.current_patch();
        liba9n::std::memcpy(info.kernel_pre_release, version.current_pre_release(), 32);
        liba9n::std::memcpy(info.kernel_build_meta_data, version.current_build_meta_data(), 32);

        // copy architectural information
        liba9n::std::memcpy(info.arch_info, boot.arch_info, sizeof(info.arch_info));

        // configure initial ipc buffer
        info.ipc_buffer = boot.boot_init_image_info.init_ipc_buffer_address;

        // configure generic information
        TRY_VOID(try_configure_init_generic_descriptors(boot.boot_memory_info, info));

        return {};
    }

    kernel_result try_configure_init_generic_descriptors(const memory_info &memory, init_info &init)
    {
        using enum memory_map_type;
        using a9n::kernel::utility::logger;
        logger::printk("Configuring init generics ...\n");
        logger::printk("Memory map: address=%p, count=0x%02x\n", memory.memory_map, memory.memory_map_count);

        if (memory.memory_map_count > INITIAL_GENERIC_COUNT_MAX)
        {
            logger::error("Memory map count is out of range!");
            return a9n::kernel::kernel_error::ILLEGAL_ARGUMENT;
        }

        for (auto memory_map_index = 0, slot_index = 0; memory_map_index < memory.memory_map_count;
             memory_map_index++)
        {
            auto entry = physical_to_virtual_pointer<a9n::kernel::memory_map_entry>(
                reinterpret_cast<a9n::physical_address>(&memory.memory_map[memory_map_index])
            );
            if (!entry)
            {
                logger::error("Memory map entry is null.");
                return a9n::kernel::kernel_error::NO_SUCH_ADDRESS;
            }

            if (entry->type == RESERVED)
            {
                continue;
            }

            auto remain = TRY(
                try_configure_generic_descriptor_from_memory_map(*entry, init.generic_list[slot_index])
            );
            slot_index++;
            init.generic_list_count = slot_index;

            // TODO: move definition to header file
            constexpr a9n::word GENERIC_RECURSIVE_SPLITTING_MAX = 7;

            for (auto i = 0; i < GENERIC_RECURSIVE_SPLITTING_MAX; i++)
            {
                if (remain.page_count == 0)
                {
                    break;
                }

                remain = TRY(try_configure_generic_descriptor_from_memory_map(
                    remain,
                    init.generic_list[slot_index]
                ));

                slot_index++;
            }
        }

        return {};
    }

    // return remained entry after splitting the largest page from the entry
    liba9n::result<memory_map_entry, kernel_error> try_configure_generic_descriptor_from_memory_map(
        const memory_map_entry &entry,
        generic_descriptor     &descriptor
    )
    {
        using a9n::kernel::utility::logger;

        if (entry.start_physical_address % a9n::PAGE_SIZE != 0)
        {
            logger::error("Memory map entry must be aligned to page size!");
            return kernel_error::ILLEGAL_ARGUMENT;
        }

        auto memory_size               = a9n::PAGE_SIZE * entry.page_count;
        auto memory_size_aligned_radix = liba9n::calculate_radix_floor(memory_size);

        auto current_generic_info      = a9n::kernel::generic_info(
            entry.start_physical_address,
            memory_size_aligned_radix,
            (entry.type == memory_map_type::DEVICE),
            entry.start_physical_address
        );

        // log
        const char *memory_status;
        switch (entry.type)
        {
            case memory_map_type::FREE :
                memory_status = "FREE";
                break;
            case memory_map_type::DEVICE :
                memory_status = "DEVICE";
                break;
            case memory_map_type::RESERVED :
                [[fallthrough]];
            default :
                memory_status = "RESERVED";
                break;
        }
        logger::printk(
            "Memory map: range=[0x%016llx-[0x%016llx | 0x%016llx]) %12s\n",
            current_generic_info.base(),
            current_generic_info.base() + memory_size,
            current_generic_info.base() + (static_cast<a9n::word>(1) << memory_size_aligned_radix),
            memory_status
        );

        auto aligned_size     = static_cast<a9n::word>(1) << memory_size_aligned_radix;

        descriptor.address    = entry.start_physical_address;
        descriptor.is_device  = (entry.type == memory_map_type::DEVICE);
        descriptor.size_radix = memory_size_aligned_radix;

        // return remain
        auto remain_address_start_raw = liba9n::align_value(
            entry.start_physical_address + (static_cast<a9n::word>(1) << memory_size_aligned_radix),
            a9n::PAGE_SIZE
        );
        auto remain_address_end_raw
            = entry.start_physical_address + (a9n::PAGE_SIZE * entry.page_count);

        auto ceiled_remain_address_start
            = liba9n::align_value(remain_address_start_raw, a9n::PAGE_SIZE);
        auto floored_remain_address_end
            = liba9n::align_value_floor(remain_address_end_raw, a9n::PAGE_SIZE);

        auto remain_size = liba9n::align_value(
            (floored_remain_address_end - ceiled_remain_address_start),
            a9n::PAGE_SIZE
        );

        if (ceiled_remain_address_start
                < (entry.start_physical_address
                   + (static_cast<a9n::word>(1) << memory_size_aligned_radix))
            || (entry.start_physical_address + (entry.page_count * a9n::PAGE_SIZE))
                   < floored_remain_address_end)
        {
            return memory_map_entry {
                .start_physical_address = 0,
                .page_count             = 0,
                .type                   = entry.type,
            };
        }
        else
        {
            return memory_map_entry {
                .start_physical_address = ceiled_remain_address_start,
                .page_count             = remain_size / a9n::PAGE_SIZE,
                .type                   = entry.type
            };
        }
    }

    kernel_result try_configure_init_process_control_block(
        capability_instance<process_control_block> &pcb,
        const init_info                            &info,
        const boot_info                            &boot
    )
    {
        using a9n::kernel::utility::logger;
        logger::printk("Configuring init process control block ...\n");

        // init hardware contexts
        logger::printk("Initializing hardware context for init process ...\n");
        hal::init_hardware_context(hal::cpu_mode::USER, pcb.component.process_core.registers);
        hal::init_floating_context(pcb.component.process_core.floating_registers);
        hal::configure_general_register(
            pcb.component.process_core,
            hal::register_type::INSTRUCTION_POINTER,
            boot.boot_init_image_info.entry_point_address
        );

        // init metadata
        logger::printk("Initializing metadata for init process ...\n");
        liba9n::std::strcpy(pcb.component.process_core.name, "INIT");

        // init basic scheduling properties
        logger::printk("Initializing scheduling properties for init process ...\n");
        pcb.component.process_core.priority = PRIORITY_MAX - 1;
        logger::printk(
            "Init process is set to the highest priority (%u) to run first.\n",
            pcb.component.process_core.priority
        );

        // finalize
        TRY_VOID(try_configure_init_io_ports(pcb.component));
        TRY_VOID(try_configure_init_address_space(pcb.component, boot, info));
        TRY_VOID(process_manager_core.mark_scheduled(pcb.component.process_core));

        return {};
    }

    kernel_result try_configure_init_io_ports(process_control_block &pcb)
    {
        using a9n::kernel::utility::logger;
        logger::printk("Configuring init IO ports ...\n");

        return pcb.process_core.root_slot.component
            ->retrieve_slot(liba9n::enum_cast(init_slot_offset::IO_PORT))
            .transform_error(
                [&]([[maybe_unused]] capability_lookup_error e) -> kernel_error
                {
                    logger::error("No slot was found to store the IO port capability!");
                    return kernel_error::NO_SUCH_ADDRESS;
                }
            )
            .and_then(
                [&](capability_slot *slot) -> kernel_result
                {
                    io_port_address_range range { .min = 0, .max = ~static_cast<a9n::word>(0) };
                    return try_configure_io_port_slot(*slot, range);
                }
            );

        return {};
    }

    kernel_result try_configure_init_address_space(
        process_control_block &pcb,
        const boot_info       &boot,
        const init_info       &init
    )
    {
        TRY_VOID(try_configure_init_root_address_space(pcb));
        TRY_VOID(try_configure_init_page_tables(pcb, boot.boot_init_image_info));
        TRY_VOID(try_configure_init_frames(pcb, boot.boot_init_image_info));
        TRY_VOID(try_configure_init_generics(pcb, init));
        TRY_VOID(try_configure_init_interrupt_region(pcb));

        return {};
    }

    kernel_result try_configure_init_root_address_space(process_control_block &pcb)
    {
        using a9n::kernel::utility::logger;

        auto make_address_space = [&](liba9n::not_null<page_size_memory> root_table_memory)
            -> liba9n::result<page_table, kernel_error>
        {
            logger::printk("Creating the root address space for Init ...\n");
            auto root_table_physical_address = virtual_to_physical_address(
                reinterpret_cast<a9n::virtual_address>(root_table_memory->data())
            );
            return hal::make_address_space(root_table_physical_address)
                .transform_error(convert_hal_to_kernel_error);
        };

        auto configure_address_space_slot = [&](page_table table) -> kernel_result
        {
            logger::printk("Configuring root address space...\n");
            capability_slot &target_slot = pcb.process_core.root_address_space;

            auto target_init_slot_result = pcb.process_core.root_slot.component->retrieve_slot(
                liba9n::enum_cast(init_slot_offset::PROCESS_ADDRESS_SPACE)
            );
            if (!target_init_slot_result)
            {
                return kernel_error::NO_SUCH_ADDRESS;
            }
            capability_slot &target_init_slot = *target_init_slot_result.unwrap();

            return try_configure_address_space_slot(target_slot, table)
                .and_then(
                    [&](void) -> kernel_result
                    {
                        logger::printk("Initializing root address space slot in init process ...\n");
                        return target_init_slot.try_remove_and_init();
                    }
                )
                .and_then(
                    [&](void) -> kernel_result
                    {
                        // return try_configure_page_table_slot(target_init_slot, table);
                        logger::printk("Copying root address space slot to init process ...\n");
                        return try_copy_capability_slot(target_init_slot, target_slot);
                    }
                );
        };

        return init_allocator.allocate<page_size_memory>(1)
            .transform_error(
                [&]([[maybe_unused]] liba9n::allocator_error e) -> kernel_error
                {
                    logger::error("Failed to allocate root page table in init process!");
                    return kernel_error::NO_SUCH_ADDRESS;
                }
            )
            .and_then(make_address_space)
            .and_then(configure_address_space_slot);
    }

    kernel_result
        try_configure_init_page_tables(process_control_block &pcb, const init_image_info &info)
    {
        using kernel::utility::logger;
        logger::printk("Configuring init page tables ...\n");

        if (!pcb.process_core.root_slot.component
            || pcb.process_core.root_slot.type != capability_type::NODE)
        {
            logger::error("Root node is invalid!");
            return kernel_error::INIT_FIRST;
        }

        auto page_node_slot_result = pcb.process_core.root_slot.component->retrieve_slot(
            liba9n::enum_cast(init_slot_offset::PROCESS_PAGE_TABLE_NODE)
        );
        if (!page_node_slot_result)
        {
            logger::error("No node was found to store the pages!");
            return kernel_error::NO_SUCH_ADDRESS;
        }

        capability_slot &page_node_slot = *page_node_slot_result.unwrap();
        if (!page_node_slot.component || page_node_slot.type != capability_type::NODE)
        {
            logger::error("Node that stores the pages is imcomplete!");
            return kernel_error::INIT_FIRST;
        }

        constexpr a9n::word FRAME_SIZE = static_cast<a9n::word>(1) << a9n::hal::INITIAL_FRAME_SIZE_BITS;
        auto root_table = convert_slot_data_to_page_table(pcb.process_core.root_address_space.data);
        a9n::word last_mapped_virtual_address = info.init_image_size * FRAME_SIZE;
        logger::printk("Last mapped virtual address : 0x%016llx\n", last_mapped_virtual_address);

        a9n::word page_table_slot_index = 0;

        auto allocate_and_map_page_table =
            [&](a9n::word depth, a9n::virtual_address map_address) -> kernel_result
        {
            DEBUG_LOG(
                "Configuring init page table: slot=%llu, depth=%llu, va=0x%016llx\n",
                page_table_slot_index,
                depth,
                map_address
            );

            return init_allocator.allocate<page_size_memory>(1)
                .transform_error(
                    [&]([[maybe_unused]] liba9n::allocator_error e) -> kernel_error
                    {
                        logger::error("Failed to allocate page table memory");
                        return kernel_error::UNEXPECTED;
                    }
                )
                .transform(
                    [&](liba9n::not_null<page_size_memory> memory) -> page_table
                    {
                        auto page_physical = virtual_to_physical_address(
                            reinterpret_cast<a9n::virtual_address>(memory->data())
                        );

                        return page_table {
                            page_physical,
                            depth,
                        };
                    }
                )
                .and_then(
                    [&](page_table table) -> liba9n::result<page_table, kernel_error>
                    {
                        return hal::map_page_table(root_table, table, map_address)
                            .transform_error(
                                [&](memory_map_error e) -> kernel_error
                                {
                                    logger::error("Failed to map page table");
                                    return kernel_error::NO_SUCH_ADDRESS;
                                }
                            )
                            .and_then(
                                [&](void) -> liba9n::result<page_table, kernel_error>
                                {
                                    return table;
                                }
                            );
                    }
                )
                .and_then(
                    [&](page_table table) -> kernel_result
                    {
                        auto target_slot_result
                            = page_node_slot.component->retrieve_slot(page_table_slot_index);
                        if (!target_slot_result)
                        {
                            logger::error("Page table slot does not exist");
                            return kernel_error::NO_SUCH_ADDRESS;
                        }

                        return try_configure_page_table_slot(*target_slot_result.unwrap(), table);
                    }
                )
                .and_then(
                    [&](void) -> kernel_result
                    {
                        page_table_slot_index++;
                        return {};
                    }
                );
        };

        for (a9n::word map_address = 0; map_address <= last_mapped_virtual_address;)
        {
            auto unset_depth_result = hal::search_unset_page_table_depth(
                root_table,
                map_address,
                a9n::hal::INITIAL_FRAME_SIZE_BITS
            );
            if (!unset_depth_result)
            {
                logger::error("Failed to search unset page table depth");
                return kernel_error::NO_SUCH_ADDRESS;
            }

            a9n::word unset_depth = unset_depth_result.unwrap();

            if (unset_depth == 0)
            {
                map_address += FRAME_SIZE;
                continue;
            }

            TRY_VOID(allocate_and_map_page_table(unset_depth, map_address));
        }

        return {};
    }

    kernel_result try_configure_init_frames(process_control_block &pcb, const init_image_info &info)
    {
        using a9n::kernel::utility::logger;
        logger::printk("Configuring init frames ...\n");

        if (!pcb.process_core.root_slot.component
            || pcb.process_core.root_slot.type != capability_type::NODE)
        {
            logger::error("Root node is imcomplete");
            return kernel_error::INIT_FIRST;
        }

        auto frame_node_slot_result = pcb.process_core.root_slot.component->retrieve_slot(
            liba9n::enum_cast(init_slot_offset::PROCESS_FRAME_NODE)
        );
        if (!frame_node_slot_result)
        {
            logger::error("No node was found to store the frame");
            return kernel_error::NO_SUCH_ADDRESS;
        }
        if (!frame_node_slot_result.unwrap()->component
            || frame_node_slot_result.unwrap()->type != capability_type::NODE)
        {
            logger::error("Node that stores the frame is imcomplete");
            return kernel_error::INIT_FIRST;
        }

        capability_slot &frame_node_slot = *frame_node_slot_result.unwrap();
        auto root_table = convert_slot_data_to_page_table(pcb.process_core.root_address_space.data);

        a9n::physical_address frame_ipc_buffer_base = info.loaded_address + info.init_ipc_buffer_address;

        constexpr a9n::word FRAME_SIZE = static_cast<a9n::word>(1) << a9n::hal::INITIAL_FRAME_SIZE_BITS;

        for (auto i = 0; i < info.init_image_size; i++)
        {
            a9n::physical_address base_address = info.loaded_address + (FRAME_SIZE * i);
            a9n::virtual_address  map_address  = FRAME_SIZE * i;

            auto target_frame
                = frame { .address = base_address, .size_bits = a9n::hal::INITIAL_FRAME_SIZE_BITS };

            auto result
                = hal::map_frame(root_table, target_frame, map_address)
                      .transform_error(
                          [&](memory_map_error e) -> kernel_error
                          {
                              logger::error("Could not map frame to init address space");
                              return kernel_error::UNEXPECTED;
                          }
                      )
                      .and_then(
                          [&](void) -> kernel_result
                          {
                              auto target_slot_result = frame_node_slot.component->retrieve_slot(i);
                              if (!target_slot_result)
                              {
                                  logger::error("Slot does not exist in the node that stores the frame");
                                  return kernel_error::NO_SUCH_ADDRESS;
                              }

                              return try_configure_frame_slot(*target_slot_result.unwrap(), target_frame)
                                  .and_then(
                                      [&](void) -> kernel_result
                                      {
                                          if (target_frame.address != frame_ipc_buffer_base)
                                          {
                                              return {};
                                          }

                                          logger::printk("Configuring IPC buffer frame ...\n");
                                          pcb.process_core.buffer
                                              = a9n::kernel::physical_to_virtual_pointer<ipc_buffer>(
                                                  frame_ipc_buffer_base
                                              );
                                          logger::printk(
                                              "Init ipc buffer: virtual address=%p\n",
                                              pcb.process_core.buffer
                                          );

                                          auto frame_ipc_buffer_slot_result
                                              = pcb.process_core.root_slot.component->retrieve_slot(
                                                  liba9n::enum_cast(init_slot_offset::PROCESS_IPC_BUFFER_FRAME)
                                              );
                                          if (!frame_ipc_buffer_slot_result)
                                          {
                                              return kernel_error::UNEXPECTED;
                                          }
                                          capability_slot &frame_ipc_buffer_slot
                                              = *frame_ipc_buffer_slot_result.unwrap();

                                          return try_configure_frame_slot(frame_ipc_buffer_slot, target_frame)
                                              .and_then(
                                                  [&](void) -> kernel_result
                                                  {
                                                      return try_configure_frame_slot(
                                                          pcb.process_core.buffer_frame,
                                                          target_frame
                                                      );
                                                  }
                                              )
                                              .and_then(
                                                  [&](void) -> kernel_result
                                                  {
                                                      target_slot_result.unwrap()->insert_sibling(
                                                          frame_ipc_buffer_slot
                                                      );
                                                      target_slot_result.unwrap()->insert_sibling(
                                                          pcb.process_core.buffer_frame
                                                      );
                                                      return {};
                                                  }
                                              );
                                      }
                                  );
                          }
                      );

            if (!result)
            {
                return result.unwrap_error();
            }
        }

        return {};
    }

    kernel_result try_configure_init_generics(process_control_block &pcb, const init_info &info)
    {
        using a9n::kernel::utility::logger;
        logger::printk("Configuring init generics ...\n");

        if (!pcb.process_core.root_slot.component
            || pcb.process_core.root_slot.type != capability_type::NODE)
        {
            logger::error("Root node is imcomplete");
            return kernel_error::INIT_FIRST;
        }

        auto generic_node_slot_result = pcb.process_core.root_slot.component->retrieve_slot(
            liba9n::enum_cast(init_slot_offset::GENERIC_NODE)
        );
        if (!generic_node_slot_result)
        {
            logger::error("No node was found to store the generic");
            return kernel_error::NO_SUCH_ADDRESS;
        }
        if (!generic_node_slot_result.unwrap()->component
            || generic_node_slot_result.unwrap()->type != capability_type::NODE)
        {
            logger::error("Node that stores the generic is imcomplete");
            return kernel_error::INIT_FIRST;
        }

        capability_slot &generic_node_slot = *generic_node_slot_result.unwrap();

        for (a9n::word i = 0; i < info.generic_list_count; i++)
        {
            const auto &generic_descriptor = info.generic_list[i];
            auto        generic_info       = a9n::kernel::generic_info(
                generic_descriptor.address,
                generic_descriptor.size_radix,
                generic_descriptor.is_device,
                generic_descriptor.address
            );
            auto generic_slot_result
                = generic_node_slot.component->retrieve_slot(i)
                      .transform_error(
                          [&]([[maybe_unused]] capability_lookup_error e) -> kernel_error
                          {
                              logger::error("Generic slot does not exist");
                              return kernel_error::NO_SUCH_ADDRESS;
                          }
                      )
                      .and_then(
                          [&](capability_slot *slot) -> liba9n::result<capability_slot *, kernel_error>
                          {
                              return slot;
                          }
                      );
            auto &generic_slot         = *generic_slot_result.unwrap();

            auto target_generic_result = try_configure_generic_slot(generic_slot, generic_info);
            if (!target_generic_result)
            {
                logger::error("Failed to configure generic slot");
                return target_generic_result.unwrap_error();
            }
        }

        return {};
    }

    kernel_result try_configure_init_interrupt_region(process_control_block &pcb)
    {
        using a9n::kernel::utility::logger;
        logger::printk("Configuring init interrupt region ...\n");

        return pcb.process_core.root_slot.component
            ->retrieve_slot(liba9n::enum_cast(init_slot_offset::INTERRUPT_REGION))
            .transform_error(
                [&]([[maybe_unused]] capability_lookup_error e) -> kernel_error
                {
                    logger::error("No slot was found to store the interrupt region!");
                    return kernel_error::NO_SUCH_ADDRESS;
                }
            )
            .and_then(
                [&](capability_slot *slot) -> kernel_result
                {
                    return try_configure_interrupt_region_slot(*slot);
                }
            );
    }
}
