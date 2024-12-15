#include "kernel/capability/page_table_capability.hpp"
#include <hal/interface/process_manager.hpp>
#include <hal/x86_64/process/process_manager.hpp>

#include <hal/arch/arch_types.hpp>
#include <hal/x86_64/arch/arch_context.hpp>
#include <hal/x86_64/arch/cpu.hpp>
#include <hal/x86_64/arch/segment_configurator.hpp>
#include <hal/x86_64/interrupt/interrupt.hpp>
#include <hal/x86_64/memory/paging.hpp>
#include <hal/x86_64/process/idle.hpp>
#include <kernel/ipc/ipc_buffer.hpp>

#include <kernel/process/process.hpp>
#include <kernel/utility/logger.hpp>

#include <liba9n/libc/string.hpp>

inline uint8_t test_stack[2048];

namespace a9n::hal::x86_64
{
}

// interface implementation
namespace a9n::hal
{

    hal_result switch_context(a9n::kernel::process &next_process)
    {
        // TODO: move page_table to hardware_context
        auto pml4 = kernel::convert_slot_data_to_page_table(next_process.root_address_space.data);
        if (!pml4.address)
        {
            a9n::kernel::utility::logger::error("no such root page table!");
            return hal_error::NO_SUCH_ADDRESS;
        }

        // a9n::kernel::utility::logger::printh("next CR3 : 0x%016llx\n", pml4);
        x86_64::_load_cr3(pml4.address);

        return {};
    }

    hal_result restore_context(cpu_mode current_mode)
    {
        using enum a9n::hal::cpu_mode;

        switch (current_mode)
        {
            case cpu_mode::KERNEL :
                x86_64::_restore_kernel_context();

            case cpu_mode::USER :
                x86_64::_restore_user_context();

            default :
                a9n::kernel::utility::logger::printh("mode 'vm' is not implemented\n");
                break;
        }

        return hal_error::UNEXPECTED;
    }

    void idle(void)
    {
        x86_64::_idle();
    }

    hal_result init_hardware_context(a9n::kernel::hardware_context &context)
    {
        liba9n::std::memset(&context, 0, a9n::hal::HARDWARE_CONTEXT_SIZE);

        context[x86_64::register_index::RFLAGS] = 0x202;
        context[x86_64::register_index::CS]     = x86_64::segment_selector::USER_CS | 0x03;
        context[x86_64::register_index::RSP] = 0; // reinterpret_cast<uint64_t>(&test_stack[2048]);
        context[x86_64::register_index::SS]  = x86_64::segment_selector::USER_DS | 0x03;

        // magic number
        context[x86_64::register_index::FS_BASE] = 0xfeedface;

        // TODO: replace this
        auto result = a9n::hal::current_local_variable().and_then(
            [&](a9n::kernel::cpu_local_variable *local_variable) -> hal_result
            {
                local_variable->current_context = &context;
                return {};
            }
        );

        return {};
    }

    // virtual message register mappings

    liba9n::result<a9n::word, hal_error>
        get_message_register(const a9n::kernel::process &target_process, a9n::word index)
    {
        // RDI : kernel call type
        // RSI : message_register_0
        // RDX : message_register_1
        // R8  : message_register_2
        // R9  : message_register_3

        a9n::word result {};

        switch (index)
        {
            case 0 :
                result = target_process.registers[x86_64::register_index::RSI];
                return result;

            case 1 :
                result = target_process.registers[x86_64::register_index::RDX];
                return result;

            case 2 :
                result = target_process.registers[x86_64::register_index::R8];
                return result;

            case 3 :
                result = target_process.registers[x86_64::register_index::R9];
                return result;

            default :
                result = target_process.buffer->get_message(index);
                return result;
        }
    }

    liba9n::result<a9n::word, hal_error>
        get_general_register(const a9n::kernel::process &target_process, register_type type)
    {
        using enum a9n::hal::register_type;

        a9n::word result {};

        switch (type)
        {
            case register_type::INSTRUCTION_POINTER :
                result = target_process.registers[x86_64::register_index::RIP];
                return result;

            case register_type::STACK_POINTER :
                result = target_process.registers[x86_64::register_index::RSP];
                return result;

            case register_type::THREAD_LOCAL_BASE :
                result = target_process.registers[x86_64::register_index::GS_BASE];
                return result;

            [[unlikely]] default :
                return hal_error::ILLEGAL_ARGUMENT;
        }
    }

    hal_result
        configure_message_register(a9n::kernel::process &target_process, a9n::word index, a9n::word value)
    {
        // RDI : kernel call type
        // RSI : message_register_0
        // RDX : message_register_1
        // R8  : message_register_2
        // R9  : message_register_3

        switch (index)
        {
            case 0 :
                target_process.registers[x86_64::register_index::RSI] = value;
                return {};

            case 1 :
                target_process.registers[x86_64::register_index::RDX] = value;
                return {};

            case 2 :
                target_process.registers[x86_64::register_index::R8] = value;
                return {};

            case 3 :
                target_process.registers[x86_64::register_index::R9] = value;
                return {};

            default :
                target_process.buffer->set_message(index, value);
                return {};
        }
    }

    hal_result
        configure_general_register(a9n::kernel::process &target_process, register_type type, a9n::word value)
    {
        using enum a9n::hal::register_type;

        switch (type)
        {
            case register_type::INSTRUCTION_POINTER :
                target_process.registers[x86_64::register_index::RIP] = value;
                break;

            case register_type::STACK_POINTER :
                target_process.registers[x86_64::register_index::RSP] = value;
                break;

            case register_type::THREAD_LOCAL_BASE :
                target_process.registers[x86_64::register_index::GS_BASE] = value;
                break;

            [[unlikely]] default :
                return hal_error::ILLEGAL_ARGUMENT;
        }

        return {};
    }
}
