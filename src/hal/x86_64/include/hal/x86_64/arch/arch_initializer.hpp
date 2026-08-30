#ifndef X86_64_ARCH_INITIALIZER_HPP
#define X86_64_ARCH_INITIALIZER_HPP

#include <hal/interface/arch_initializer.hpp>

namespace a9n::hal::x86_64
{
    hal_result init_architecture_impl(a9n::word arch_info[]);
    hal_result init_main_core(a9n::physical_address rsdp_address);
}

#endif
