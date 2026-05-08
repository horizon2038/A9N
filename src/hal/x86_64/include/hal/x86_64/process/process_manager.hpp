#ifndef X86_64_CONTEXT_SWITCH_HPP
#define X86_64_CONTEXT_SWITCH_HPP

#include <hal/hal_result.hpp>
#include <hal/interface/process_manager.hpp>
#include <kernel/types.hpp>

#include <hal/x86_64/arch/segment_configurator.hpp>
#include <hal/x86_64/memory/paging.hpp>

namespace a9n::hal::x86_64
{
    inline constexpr a9n::word MESSAGE_REGISTER_SIZE_MAX = 9;

    // unused
    // TODO: remove this
    extern "C" void _switch_context(
        a9n::virtual_address *preview_stack_pointer,
        a9n::virtual_address *next_stack_pointer
    );

}

#endif
