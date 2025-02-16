#include "kernel/ipc/ipc_buffer.hpp"
#include <kernel/boot/init.hpp>

#include <kernel/capability/address_space.hpp>
#include <kernel/capability/capability_component.hpp>
#include <kernel/capability/capability_node.hpp>
#include <kernel/capability/frame_capability.hpp>
#include <kernel/capability/generic.hpp>
#include <kernel/capability/page_table_capability.hpp>
#include <kernel/capability/process_control_block.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/memory/memory_type.hpp>

#include <kernel/memory/memory.hpp>
#include <kernel/process/process.hpp>
#include <kernel/process/process_manager.hpp>
#include <kernel/types.hpp>

#include <kernel/process/scheduler.hpp>
#include <kernel/utility/logger.hpp>

#include <hal/interface/memory_manager.hpp>
#include <hal/interface/process_manager.hpp>

#include <liba9n/common/allocator.hpp>
#include <liba9n/common/calculate.hpp>
#include <liba9n/common/enum.hpp>
#include <liba9n/common/not_null.hpp>

// TODO: CLEAN IT BECAUSE IT IS TOO CONFUSING !!!!!

namespace a9n::kernel
{
    using page_size_memory = liba9n::std::array<uint8_t, a9n::PAGE_SIZE>;

    // assign only once; no memory freed
    liba9n::linear_allocator<a9n::PAGE_SIZE * 32> init_allocator {};

    static liba9n::result<liba9n::not_null<init_info>, kernel_error>
                         try_make_init_info(const boot_info &info);
    static kernel_result try_make_init_nodes(capability_slot &slot, a9n::word count);
    static liba9n::result<liba9n::not_null<process_control_block>, kernel_error>
        try_make_init_process_control_block(const boot_info &info, init_info &init_info_page);
    static kernel_result try_make_init_process_root_address_space(process_control_block &pcb);
    static kernel_result
        try_make_init_process_pages(const init_image_info &info, process_control_block &pcb);
    static kernel_result try_make_init_process_frames(
        const init_image_info &info,
        process_control_block &pcb,
        init_info             &init_info_page
    );
    static kernel_result try_make_init_generics(
        const memory_info &info,
        capability_slot   &node_slot,
        init_info         &init_info_page
    );
    static kernel_result create_init_interrupt_port();

    static liba9n::result<memory_map_entry, kernel_error> try_make_generic_from_memory_map(
        const memory_map_entry &entry,
        capability_slot        &slot,
        generic_descriptor     &descriptor
    );

    kernel_result create_init(const boot_info &info)
    {
        using a9n::kernel::utility::logger;

        // create init_info from raw address
        logger::printk("create init info from raw address ...\n");
        auto init_info_result = try_make_init_info(info);
        if (!init_info_result)
        {
            return init_info_result.unwrap_error();
        }
        auto init_info_page = init_info_result.unwrap();

        // create init : process control block
        logger::printk("create init process control block ...\n");
        auto init_process_control_block_result
            = try_make_init_process_control_block(info, init_info_page.get());
        if (!init_process_control_block_result)
        {
            return init_process_control_block_result.unwrap_error();
        }
        auto init_process_control_block = init_process_control_block_result.unwrap();

        // create init : ipc buffer frame
        auto init_ipc_buffer_slot_result
            = init_process_control_block->process_core.root_slot.component->retrieve_slot(
                liba9n::enum_cast(init_slot_offset::PROCESS_IPC_BUFFER_FRAME)
            );

        // create init : node for storing generic
        auto generic_node_slot_result
            = init_process_control_block->process_core.root_slot.component->retrieve_slot(
                liba9n::enum_cast(init_slot_offset::GENERIC_NODE)
            );
        if (!generic_node_slot_result)
        {
            logger::error("capability node for generic storage is not set");
            return kernel_error::INIT_FIRST;
        }
        capability_slot &generic_node_slot = *generic_node_slot_result.unwrap();

        logger::printk("create init generic node ...\n");
        auto generic_node_result = try_make_init_nodes(generic_node_slot, INITIAL_GENERIC_COUNT_MAX);
        if (!generic_node_result)
        {
            generic_node_result.unwrap_error();
        }

        // create init : create generic
        logger::printk("create init generic entries ...\n");

        // TODO: configure generic node slots : func(meminfo, &generic_node_slot) -> kresult
        auto generic_make_result
            = try_make_init_generics(info.boot_memory_info, generic_node_slot, init_info_page.get());
        if (!generic_make_result)
        {
            return generic_make_result.unwrap_error();
        }

        // create interrupt region

        // create io port

        return {};
    }

    static liba9n::result<liba9n::not_null<init_info>, kernel_error>
        try_make_init_info(const boot_info &info)
    {
        a9n::physical_address init_info_address
            = info.boot_init_image_info.loaded_address + info.boot_init_image_info.init_info_address;
        if (!init_info_address)
        {
            return kernel_error::NO_SUCH_ADDRESS;
        }

        init_info &init_info_page
            = *a9n::kernel::physical_to_virtual_pointer<init_info>(init_info_address);

        // copy architectural information
        liba9n::std::memcpy(init_info_page.arch_info, info.arch_info, sizeof(info.arch_info));

        return liba9n::not_null<init_info> { init_info_page };
    }

    static kernel_result try_make_init_nodes(capability_slot &slot, a9n::word count)
    {
        auto slots_result = init_allocator.allocate<capability_slot>(count);
        if (!slots_result)
        {
            return kernel_error::UNEXPECTED;
        }
        auto slots = slots_result.unwrap();

        // create root node
        auto init_node_result = init_allocator.allocate<a9n::kernel::capability_node>(
            1,
            0,
            liba9n::calculate_radix_floor(count),
            &slots.get()
        );
        if (!init_node_result)
        {
            return kernel_error::UNEXPECTED;
        }
        auto init_node = init_node_result.unwrap();

        // configure node
        // slot.component = &(init_node.get());
        // slot.type      = capability_type::NODE;
        //
        return try_configure_capability_node_slot(slot, init_node.get());
    }

    static liba9n::result<liba9n::not_null<process_control_block>, kernel_error>
        try_make_init_process_control_block(const boot_info &info, init_info &init_info_page)
    {
        using kernel::utility::logger;

        auto init_process_control_block_result = init_allocator.allocate<process_control_block>(1);
        if (!init_process_control_block_result)
        {
            return kernel_error::UNEXPECTED;
        }
        auto init_process_control_block = init_process_control_block_result.unwrap();

        // configure process control block
        hal::init_hardware_context(hal::cpu_mode::USER, init_process_control_block->process_core.registers);
        hal::configure_general_register(
            init_process_control_block->process_core,
            hal::register_type::INSTRUCTION_POINTER,
            info.boot_init_image_info.entry_point_address
        );

        // make  root node
        logger::printk("try to make the root node in init process ...\n");
        init_process_control_block->process_core.root_slot.type = capability_type::NONE;
        auto root_node_result
            = try_make_init_nodes(
                  init_process_control_block->process_core.root_slot,
                  INITIAL_PROCESS_ROOT_NODE_COUNT
            )
                  .and_then(
                      [&](void) -> kernel_result
                      {
                          return init_process_control_block->process_core.root_slot.component
                              ->retrieve_slot(liba9n::enum_cast(init_slot_offset::PROCESS_ROOT_NODE))
                              .transform_error(
                                  [&]([[maybe_unused]] capability_lookup_error e) -> kernel_error
                                  {
                                      return kernel_error::NO_SUCH_ADDRESS;
                                  }
                              )
                              .and_then(
                                  [&](capability_slot *slot) -> kernel_result
                                  {
                                      return try_copy_capability_slot(
                                          *slot,
                                          init_process_control_block->process_core.root_slot
                                      );
                                  }
                              );
                      }
                  );
        if (!root_node_result)
        {
            return root_node_result.unwrap_error();
        }

        // configure memories
        capability_slot &root_slot = init_process_control_block->process_core.root_slot;

        // page slots
        auto page_node_slot_result = root_slot.component->retrieve_slot(
            liba9n::enum_cast(init_slot_offset::PROCESS_PAGE_TABLE_NODE)
        );
        if (!page_node_slot_result)
        {
            return kernel_error::NO_SUCH_ADDRESS;
        }
        capability_slot &page_node_slot = *page_node_slot_result.unwrap();

        auto page_node_result = try_make_init_nodes(page_node_slot, INITIAL_PAGE_TABLE_COUNT_MAX);
        if (!page_node_result)
        {
            logger::error("failed to make the page node in init process");
            return root_node_result.unwrap_error();
        }

        // frame slots
        auto frame_slot_result = root_slot.component->retrieve_slot(
            liba9n::enum_cast(init_slot_offset::PROCESS_FRAME_NODE)
        );
        if (!frame_slot_result)
        {
            return kernel_error::NO_SUCH_ADDRESS;
        }
        capability_slot &frame_slot = *frame_slot_result.unwrap();

        auto frame_result           = try_make_init_nodes(frame_slot, INITIAL_FRAME_COUNT_MAX);
        if (!frame_result)
        {
            logger::error("failed to make the frame node in init process");
            return root_node_result.unwrap_error();
        }

        return try_make_init_process_root_address_space(init_process_control_block.get())
            .and_then(
                [&](void) -> kernel_result
                {
                    return try_make_init_process_pages(
                        info.boot_init_image_info,
                        init_process_control_block.get()
                    );
                }
            )
            .and_then(
                [&](void) -> kernel_result
                {
                    return try_make_init_process_frames(
                        info.boot_init_image_info,
                        init_process_control_block.get(),
                        init_info_page
                    );
                }
            )
            .and_then(
                [&](void) -> liba9n::result<liba9n::not_null<process_control_block>, kernel_error>
                {
                    liba9n::std::strcpy(init_process_control_block->process_core.name, "INIT");

                    init_process_control_block->process_core.status = process_status::READY;
                    process_manager_core.mark_scheduled(init_process_control_block->process_core);

                    return init_process_control_block;
                }
            );
    }

    static kernel_result try_make_init_process_root_address_space(process_control_block &pcb)
    {
        using a9n::kernel::utility::logger;
        logger::printk("try to make the root address space in init process ...\n");

        return init_allocator.allocate<page_size_memory>(1)
            .transform_error(
                [&]([[maybe_unused]] liba9n::allocator_error e) -> kernel_error
                {
                    logger::error("failed to allocate root page table in init process!");
                    return kernel_error::NO_SUCH_ADDRESS;
                }
            )
            .and_then(
                [&](liba9n::not_null<page_size_memory> root_table_memory
                ) -> liba9n::result<page_table, kernel_error>
                {
                    auto root_table_physical_address = virtual_to_physical_address(
                        reinterpret_cast<a9n::virtual_address>(root_table_memory->data())
                    );
                    return hal::make_address_space(root_table_physical_address)
                        .transform_error(convert_hal_to_kernel_error);
                }
            )
            .and_then(
                [&](page_table table) -> kernel_result
                {
                    logger::printk("init root : 0x%016llx (depth %04llu)\n", table.address, table.depth);

                    capability_slot &target_slot = pcb.process_core.root_address_space;

                    auto target_init_slot_result = pcb.process_core.root_slot.component->retrieve_slot(
                        liba9n::enum_cast(init_slot_offset::PROCESS_ADDRESS_SPACE)
                    );
                    if (!target_init_slot_result)
                    {
                        return kernel_error::NO_SUCH_ADDRESS;
                    }
                    capability_slot &target_init_slot = *target_init_slot_result.unwrap();

                    DEBUG_LOG("try to configure address space slot\n");
                    return try_configure_address_space_slot(target_slot, table)
                        .and_then(
                            [&](void) -> kernel_result
                            {
                                // return try_configure_page_table_slot(target_init_slot, table);
                                DEBUG_LOG("try to inti root address slot\n");
                                return target_init_slot.try_remove_and_init();
                            }
                        )
                        .and_then(
                            [&](void) -> kernel_result
                            {
                                // return try_configure_page_table_slot(target_init_slot, table);
                                DEBUG_LOG("try to copy address space slot\n");
                                return try_copy_capability_slot(target_init_slot, target_slot);
                            }
                        );
                }
            );
    }

    static kernel_result
        try_make_init_process_pages(const init_image_info &info, process_control_block &pcb)
    {
        using kernel::utility::logger;
        logger::printk("try to make the pages in init process ...\n");

        if (!pcb.process_core.root_slot.component)
        {
            logger::error("the node that stores the frame is imcomplete");
            return kernel_error::INIT_FIRST;
        }

        auto root_table = convert_slot_data_to_page_table(pcb.process_core.root_address_space.data);
        return hal::search_unset_page_table_depth(root_table, 0)
            .transform_error(
                [&](memory_map_error e) -> kernel_error
                {
                    logger::error("failed to map search unset table depth");
                    return kernel_error::NO_SUCH_ADDRESS;
                }
            )
            .and_then(
                [&](a9n::word depth) -> kernel_result
                {
                    logger::printk("number of pages to be created : %04d\n", depth);

                    for (a9n::word slot_index = 0, page_depth = depth; slot_index < depth;
                         slot_index++, page_depth--)
                    {
                        DEBUG_LOG("slot_index : %llu | pade_depth : %llu\n", slot_index, page_depth);
                        auto result
                            = init_allocator.allocate<page_size_memory>(1)
                                  .transform_error(
                                      [&]([[maybe_unused]] liba9n::allocator_error e) -> kernel_error
                                      {
                                          return kernel_error::UNEXPECTED;
                                      }
                                  )
                                  .and_then(
                                      [&](liba9n::not_null<page_size_memory> memory
                                      ) -> liba9n::result<page_table, kernel_error>
                                      {
                                          auto page_physical = virtual_to_physical_address(
                                              reinterpret_cast<a9n::virtual_address>(memory->data())
                                          );
                                          return page_table { .address = page_physical,
                                                              .depth   = page_depth };
                                      }
                                  )
                                  .and_then(
                                      [&](page_table table) -> liba9n::result<page_table, kernel_error>
                                      {
                                          return hal::map_page_table(root_table, table, 0)
                                              .transform_error(
                                                  [&](memory_map_error e) -> kernel_error
                                                  {
                                                      logger::error("failed to map root table");
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
                                          return pcb.process_core.root_slot.component
                                              ->retrieve_slot(liba9n::enum_cast(
                                                  init_slot_offset::PROCESS_PAGE_TABLE_NODE
                                              ))
                                              .transform_error(
                                                  [&]([[maybe_unused]] capability_lookup_error e
                                                  ) -> kernel_error
                                                  {
                                                      logger::error("failed to lookup node");
                                                      return kernel_error::UNEXPECTED;
                                                  }
                                              )
                                              .and_then(
                                                  [&](capability_slot *slot) -> kernel_result
                                                  {
                                                      if (!slot->component
                                                          || slot->type != capability_type::NODE)
                                                      {
                                                          logger::error("the node that stores the "
                                                                        "pages is imcomplete");
                                                          return kernel_error::INIT_FIRST;
                                                      }

                                                      auto target_slot_result
                                                          = slot->component->retrieve_slot(slot_index);
                                                      if (!target_slot_result)
                                                      {
                                                          return kernel_error::NO_SUCH_ADDRESS;
                                                      }
                                                      return try_configure_page_table_slot(
                                                          *target_slot_result.unwrap(),
                                                          table
                                                      );
                                                  }
                                              );
                                      }
                                  );

                        if (!result)
                        {
                            logger::error("failed to create init pages");
                            return kernel_error::UNEXPECTED;
                        }
                    }

                    return {};
                }
            );
    }

    static kernel_result try_make_init_process_frames(
        const init_image_info &info,
        process_control_block &pcb,
        init_info             &init_info_page
    )
    {
        using a9n::kernel::utility::logger;
        logger::printk("try to make the frames in init process ...\n");

        if (!pcb.process_core.root_slot.component
            || pcb.process_core.root_slot.type != capability_type::NODE)
        {
            logger::error("the root node is imcomplete");
            return kernel_error::INIT_FIRST;
        }

        auto frame_node_slot_result = pcb.process_core.root_slot.component->retrieve_slot(
            liba9n::enum_cast(init_slot_offset::PROCESS_FRAME_NODE)
        );
        if (!frame_node_slot_result)
        {
            logger::error("no node was found to store the frame");
            return kernel_error::NO_SUCH_ADDRESS;
        }
        if (!frame_node_slot_result.unwrap()->component
            || frame_node_slot_result.unwrap()->type != capability_type::NODE)
        {
            logger::error("the node that stores the frame is imcomplete");
            return kernel_error::INIT_FIRST;
        }

        capability_slot &frame_node_slot = *frame_node_slot_result.unwrap();
        auto root_table = convert_slot_data_to_page_table(pcb.process_core.root_address_space.data);

        a9n::physical_address frame_ipc_buffer_base = info.loaded_address + info.init_ipc_buffer_address;

        // create frames
        for (auto i = 0; i <= info.init_image_size; i++)
        {
            a9n::physical_address base_address = info.loaded_address + (a9n::PAGE_SIZE * i);
            a9n::virtual_address  map_address  = a9n::PAGE_SIZE * i;
            auto                  target_frame = frame { .address = base_address };

            auto result
                = hal::map_frame(root_table, target_frame, map_address)
                      .transform_error(
                          [&](memory_map_error e) -> kernel_error
                          {
                              logger::error("could not map frame to init address space");
                              return kernel_error::UNEXPECTED;
                          }
                      )
                      .and_then(
                          [&](void) -> kernel_result
                          {
                              auto target_slot_result = frame_node_slot.component->retrieve_slot(i);
                              if (!target_slot_result)
                              {
                                  DEBUG_LOG(
                                      "slot error code : 0x%016llx",
                                      static_cast<a9n::word>(target_slot_result.unwrap_error())
                                  );
                                  logger::error("slot does not exist in the node that stores the "
                                                "frame");
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

                                          logger::printk("try make ipc buffer frame ...\n");

                                          init_info_page.ipc_buffer = info.init_ipc_buffer_address;
                                          pcb.process_core.buffer
                                              = a9n::kernel::physical_to_virtual_pointer<ipc_buffer>(
                                                  frame_ipc_buffer_base
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
                                                      // register sibling
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

        init_info_page.kernel_version = 0xdeadc0de;

        return {};
    }

    static kernel_result
        try_make_init_generics(const memory_info &info, capability_slot &node_slot, init_info &init_info_page)
    {
        using enum memory_map_type;

        if (info.memory_map_count > INITIAL_GENERIC_COUNT_MAX)
        {
            a9n::kernel::utility::logger::printk(
                "memory_map_count [%4llu] is out of range\n",
                info.memory_map_count
            );
            return a9n::kernel::kernel_error::ILLEGAL_ARGUMENT;
        }

        for (auto memory_map_index = 0, slot_index = 0; memory_map_index < info.memory_map_count;
             memory_map_index++)
        {
            a9n::kernel::utility::logger::printk("[%4llu] generic initialization\n", memory_map_index);
            // a9n::kernel::memory_map_entry *entry = &info.memory_map[memory_map_index];
            auto entry = physical_to_virtual_pointer<a9n::kernel::memory_map_entry>(
                reinterpret_cast<a9n::physical_address>(&info.memory_map[memory_map_index])
            );
            if (!entry)
            {
                a9n::kernel::utility::logger::error("entry is null");
                return a9n::kernel::kernel_error::NO_SUCH_ADDRESS;
            }

            if (entry->type == RESERVED)
            {
                continue;
            }

            a9n::kernel::utility::logger::printk("create generic info ...\n");

            a9n::kernel::utility::logger::printk("configure generic slots data ...\n");
            auto target_slot_result = node_slot.component->retrieve_slot(slot_index);
            if (!target_slot_result)
            {
                return kernel_error::NO_SUCH_ADDRESS;
            }
            auto target_slot = target_slot_result.unwrap();

            auto base_result = try_make_generic_from_memory_map(
                *entry,
                *target_slot,
                init_info_page.generic_list[slot_index]
            );
            if (!base_result)
            {
                return base_result.unwrap_error();
            }

            slot_index++;
            init_info_page.generic_list_count                   = slot_index;

            constexpr a9n::word GENERIC_RECURSIVE_SPLITTING_MAX = 7;

            memory_map_entry &remain                            = base_result.unwrap();

            for (auto i = 0; i < GENERIC_RECURSIVE_SPLITTING_MAX; i++)
            {
                if (remain.page_count == 0)
                {
                    break;
                }

                a9n::kernel::utility::logger::printk("re-splitting [%2d] ...\n", i);

                auto remain_slot_result = node_slot.component->retrieve_slot(slot_index);
                if (!remain_slot_result)
                {
                    return kernel_error::NO_SUCH_ADDRESS;
                }
                auto remain_slot = remain_slot_result.unwrap();

                auto res         = try_make_generic_from_memory_map(
                    remain,
                    *remain_slot,
                    init_info_page.generic_list[slot_index]
                );
                if (!res)
                {
                    return res.unwrap_error();
                }

                remain = res.unwrap();

                slot_index++;
            }
        }

        init_info_page.generic_start
            = liba9n::enum_cast(init_slot_offset::GENERIC_NODE)
           << ((a9n::WORD_BITS - 1) - a9n::BYTE_BITS
               - liba9n::calculate_radix_floor(INITIAL_GENERIC_COUNT_MAX));

        return {};
    }

    static liba9n::result<memory_map_entry, kernel_error> try_make_generic_from_memory_map(
        const memory_map_entry &entry,
        capability_slot        &slot,
        generic_descriptor     &descriptor
    )
    {
        if (entry.start_physical_address % a9n::PAGE_SIZE != 0)
        {
            a9n::kernel::utility::logger::error("memory map entry must be aligned to page size");
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
                memory_status = "FREE MEMORY";
                break;
            case memory_map_type::DEVICE :
                memory_status = "DEVICE MEMORY";
                break;
            case memory_map_type::RESERVED :
                [[fallthrough]];
            default :
                memory_status = "RESERVED MEMORY";
                break;
        }
        a9n::kernel::utility::logger::printk(
            "raw    : [0x%016llx - 0x%016llx) : %s\n",
            current_generic_info.base(),
            current_generic_info.base() + memory_size,
            memory_status
        );
        a9n::kernel::utility::logger::printk(
            "actual : [0x%016llx - 0x%016llx) : %s\n",
            current_generic_info.base(),
            current_generic_info.base() + (static_cast<a9n::word>(1) << memory_size_aligned_radix),
            memory_status
        );

        return try_configure_generic_slot(slot, current_generic_info)
            .and_then(
                [&](void) -> kernel_result
                {
                    auto aligned_size     = static_cast<a9n::word>(1) << memory_size_aligned_radix;

                    descriptor.address    = entry.start_physical_address;
                    descriptor.is_device  = (entry.type == memory_map_type::DEVICE);
                    descriptor.size_radix = memory_size_aligned_radix;

                    return {};
                }
            )
            .and_then(
                [&](void) -> liba9n::result<memory_map_entry, kernel_error>
                {
                    // return remain
                    auto remain_address_start_raw = liba9n::align_value(
                        entry.start_physical_address
                            + (static_cast<a9n::word>(1) << memory_size_aligned_radix),
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
                        || (entry.start_physical_address + (entry.page_count * a9n::PAGE_SIZE)
                           ) < floored_remain_address_end)
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
            );
    }

}
