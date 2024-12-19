#ifndef X86_64_ARCH_INITIALIZER_HPP
#define X86_64_ARCH_INITIALIZER_HPP

#include "kernel/process/cpu.hpp"
#include "kernel/types.hpp"
#include <hal/interface/arch_initializer.hpp>

namespace a9n::hal::x86_64
{
    class arch_initializer final : public a9n::hal::arch_initializer
    {
      public:
        hal_result init_architecture(a9n::word arch_info[]) override;
    };

    hal_result init_main_core(a9n::physical_address rsdp_address);

    inline arch_initializer arch_initializer_core {};
}

#endif
