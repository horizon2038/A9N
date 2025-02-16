#include <hal/interface/process_manager.hpp>

#include <hal/arch/arch_types.hpp>
#include <hal/x86_64/arch/arch_context.hpp>
#include <hal/x86_64/arch/control_register.hpp>
#include <hal/x86_64/arch/cpu.hpp>
#include <hal/x86_64/arch/floating_point.hpp>
#include <hal/x86_64/arch/segment_configurator.hpp>
#include <hal/x86_64/interrupt/interrupt.hpp>
#include <hal/x86_64/memory/paging.hpp>
#include <hal/x86_64/process/idle.hpp>
#include <hal/x86_64/process/process_manager.hpp>

#include <kernel/capability/page_table_capability.hpp>
#include <kernel/ipc/ipc_buffer.hpp>
#include <kernel/process/process.hpp>
#include <kernel/utility/logger.hpp>

#include <liba9n/libc/string.hpp>

namespace a9n::hal::x86_64
{
}

// interface implementation
namespace a9n::hal
{
    namespace
    {

    }

    hal_result switch_context(a9n::kernel::process &preview_process, a9n::kernel::process &next_process)
    {
        x86_64::switch_floating_context(
            preview_process.floating_registers,
            next_process.floating_registers
        );

        a9n::virtual_address current_cr3;

        asm volatile("mov %%cr3, %0" : "=r"(current_cr3) : :);

        auto next_pml4 = kernel::convert_slot_data_to_page_table(next_process.root_address_space.data);
        if (!next_pml4.address) [[unlikely]]
        {
            a9n::kernel::utility::logger::error("no such root page table!");
            return hal_error::NO_SUCH_ADDRESS;
        }

        // if address space is the same, TLB flush is no necessary
        if (current_cr3 == next_pml4.address)
        {
            return {};
        }

        x86_64::_load_cr3(next_pml4.address);

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

    hal_result init_hardware_context(cpu_mode mode, a9n::kernel::hardware_context &context)
    {
        liba9n::std::memset(&context, 0, a9n::hal::HARDWARE_CONTEXT_SIZE);

        switch (mode)
        {
            case cpu_mode::KERNEL :
                {
                    context[x86_64::register_index::RFLAGS] = 0x202;
                    context[x86_64::register_index::CS]     = x86_64::segment_selector::KERNEL_CS;
                    context[x86_64::register_index::RSP]    = 0;
                    context[x86_64::register_index::SS]     = x86_64::segment_selector::KERNEL_DS;

                    // magic number (for debugging)
                    context[x86_64::register_index::FS_BASE] = 0xdeadc0de;

                    break;
                }

            case cpu_mode::USER :
                {
                    context[x86_64::register_index::RFLAGS] = 0x202;
                    context[x86_64::register_index::CS]  = x86_64::segment_selector::USER_CS | 0x03;
                    context[x86_64::register_index::RSP] = 0;
                    context[x86_64::register_index::SS]  = x86_64::segment_selector::USER_DS | 0x03;

                    // magic number (for debugging)
                    context[x86_64::register_index::FS_BASE] = 0xfeedface;

                    break;
                }

            default :
                return hal_error::ILLEGAL_ARGUMENT;
        }

        return {};
    }

    // virtual message register mappings

    liba9n::result<a9n::word, hal_error>
        get_message_register(const a9n::kernel::process &target_process, a9n::word index)
    {
        // RSI : message_register[0]
        // RDX : message_register[1]
        // R8  : message_register[2]
        // R9  : message_register[3]
        // R10 : message_register[4]
        // R12 : message_register[5]
        // R13 : message_register[6]
        // R14 : message_register[7]
        // R15 : message_register[8]

        a9n::word result {};

        switch (index)
        {
            case 0 :
                return static_cast<a9n::word>(target_process.registers[x86_64::register_index::RSI]);

            case 1 :
                return static_cast<a9n::word>(target_process.registers[x86_64::register_index::RDX]);

            case 2 :
                return static_cast<a9n::word>(target_process.registers[x86_64::register_index::R8]);

            case 3 :
                return static_cast<a9n::word>(target_process.registers[x86_64::register_index::R9]);

            case 4 :
                return static_cast<a9n::word>(target_process.registers[x86_64::register_index::R10]);

            case 5 :
                return static_cast<a9n::word>(target_process.registers[x86_64::register_index::R12]);

            case 6 :
                return static_cast<a9n::word>(target_process.registers[x86_64::register_index::R13]);

            case 7 :
                return static_cast<a9n::word>(target_process.registers[x86_64::register_index::R14]);

            case 8 :
                return static_cast<a9n::word>(target_process.registers[x86_64::register_index::R15]);

            default :
                {
                    if (!target_process.buffer) [[unlikely]]
                    {
                        DEBUG_LOG("no such IPC buffer");
                        return hal_error::NO_SUCH_ADDRESS;
                    }

                    return target_process.buffer->get_message(index);
                }
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
        // RSI : message_register[0]
        // RDX : message_register[1]
        // R8  : message_register[2]
        // R9  : message_register[3]
        // R10 : message_register[4]
        // R12 : message_register[5]
        // R13 : message_register[6]
        // R14 : message_register[7]
        // R15 : message_register[8]

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

            case 4 :
                target_process.registers[x86_64::register_index::R10] = value;
                return {};

            case 5 :
                target_process.registers[x86_64::register_index::R12] = value;
                return {};

            case 6 :
                target_process.registers[x86_64::register_index::R13] = value;
                return {};

            case 7 :
                target_process.registers[x86_64::register_index::R14] = value;
                return {};

            case 8 :
                target_process.registers[x86_64::register_index::R15] = value;
                return {};

            default :
                [[unlikely]] if (!target_process.buffer)
                {
                    DEBUG_LOG("no such IPC buffer");
                    return hal_error::NO_SUCH_ADDRESS;
                }

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

    bool is_valid_user_address(a9n::virtual_address address)
    {
        // is lower-half address?
        return (0 <= address && address < 0x0000'7fff'ffff'ffff);
    }
}
