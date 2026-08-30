#include <hal/interface/process_manager.hpp>

#include <hal/aarch64/arch/arch_context.hpp>
#include <hal/aarch64/memory/paging.hpp>

#include <kernel/capability/page_table_capability.hpp>
#include <kernel/config.hpp>
#include <kernel/ipc/ipc_buffer.hpp>
#include <kernel/process/cpu.hpp>
#include <kernel/utility/logger.hpp>

#include <liba9n/libc/string.hpp>

namespace a9n::hal
{
    namespace
    {
        constexpr a9n::word message_register_index(a9n::word index)
        {
            // x8 carries the kernel-call number, so MR8 and MR9 use x9 and x10.
            return index < 8 ? index : index + 1;
        }
    }

    hal_result switch_context(a9n::kernel::process &preview_process, a9n::kernel::process &next_process)
    {
        aarch64::aarch64_save_floating_context(preview_process.floating_registers.data());
        aarch64::aarch64_restore_floating_context(next_process.floating_registers.data());

        auto next_root = kernel::convert_slot_data_to_page_table(next_process.root_address_space.data);
        if (!next_root.address)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }

        const auto current_root = aarch64::current_user_page_table();
        if (current_root != next_root.address)
        {
            const auto owner = static_cast<a9n::word>(1)
                            << (next_process.core_affinity & (a9n::WORD_BITS - 1));
            if constexpr (kernel::SMP_ENABLED)
            {
                write_address_space_owners(next_root, read_address_space_owners(next_root) | owner);
            }

            aarch64::load_user_page_table(next_root.address);

            if constexpr (kernel::SMP_ENABLED)
            {
                if (current_root)
                {
                    auto previous_root
                        = a9n::kernel::page_table { current_root, aarch64::page_depth::L0 };
                    write_address_space_owners(
                        previous_root,
                        read_address_space_owners(previous_root) & ~owner
                    );
                }
            }
        }

        return {};
    }

    hal_result restore_context(cpu_mode current_mode)
    {
        if (current_mode != cpu_mode::KERNEL && current_mode != cpu_mode::USER)
        {
            return hal_error::ILLEGAL_ARGUMENT;
        }

        auto local_result = current_local_variable();
        if (!local_result)
        {
            return local_result.unwrap_error();
        }

        auto *context = local_result.unwrap()->current_context;
        if (!context)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }
        aarch64::aarch64_restore_context(context->data());
    }

    [[noreturn]] void idle()
    {
        for (;;)
        {
            asm volatile("wfi" ::: "memory");
        }
    }

    hal_result init_hardware_context(cpu_mode mode, a9n::kernel::hardware_context &context)
    {
        liba9n::std::memset(&context, 0, sizeof(context));
        switch (mode)
        {
            case cpu_mode::KERNEL :
                context[aarch64::register_index::SPSR_EL1] = aarch64::SPSR_MODE_EL1H;
                return {};
            case cpu_mode::USER :
                context[aarch64::register_index::SPSR_EL1] = aarch64::SPSR_MODE_EL0T;
                return {};
            default :
                return hal_error::ILLEGAL_ARGUMENT;
        }
    }

    hal_result init_floating_context(a9n::kernel::floating_context &context)
    {
        liba9n::std::memset(&context, 0, sizeof(context));
        return {};
    }

    liba9n::result<a9n::word, hal_error>
        get_message_register(const a9n::kernel::process &target_process, a9n::word index)
    {
        if (index < 10)
        {
            return static_cast<a9n::word>(target_process.registers[message_register_index(index)]);
        }
        if (!target_process.buffer)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }
        return target_process.buffer->get_message(index);
    }

    liba9n::result<a9n::word, hal_error>
        get_general_register(const a9n::kernel::process &target_process, register_type type)
    {
        switch (type)
        {
            case register_type::INSTRUCTION_POINTER :
                return static_cast<a9n::word>(
                    target_process.registers[aarch64::register_index::ELR_EL1]
                );
            case register_type::STACK_POINTER :
                return static_cast<a9n::word>(target_process.registers[aarch64::register_index::SP_EL0]);
            case register_type::THREAD_LOCAL_BASE :
                return static_cast<a9n::word>(
                    target_process.registers[aarch64::register_index::TPIDR_EL0]
                );
            default :
                return hal_error::ILLEGAL_ARGUMENT;
        }
    }

    hal_result
        configure_message_register(a9n::kernel::process &target_process, a9n::word index, a9n::word value)
    {
        if (index < 10)
        {
            target_process.registers[message_register_index(index)] = value;
            return {};
        }
        if (!target_process.buffer)
        {
            return hal_error::NO_SUCH_ADDRESS;
        }
        target_process.buffer->set_message(index, value);
        return {};
    }

    hal_result
        configure_general_register(a9n::kernel::process &target_process, register_type type, a9n::word value)
    {
        switch (type)
        {
            case register_type::INSTRUCTION_POINTER :
                target_process.registers[aarch64::register_index::ELR_EL1] = value;
                return {};
            case register_type::STACK_POINTER :
                target_process.registers[aarch64::register_index::SP_EL0] = value;
                return {};
            case register_type::THREAD_LOCAL_BASE :
                target_process.registers[aarch64::register_index::TPIDR_EL0] = value;
                return {};
            default :
                return hal_error::ILLEGAL_ARGUMENT;
        }
    }

    bool is_valid_user_address(a9n::virtual_address address)
    {
        return address < 0x0000800000000000ULL;
    }
}
