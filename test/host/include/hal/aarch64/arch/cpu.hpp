#pragma once
#define invalidate_tlb_all unused_privileged_invalidate_tlb_all
#include "../../../../../../src/hal/aarch64/include/hal/aarch64/arch/cpu.hpp"
#undef invalidate_tlb_all
#include <test_machine.hpp>

namespace a9n::hal::aarch64
{
    inline void invalidate_tlb_all() { ++test_machine::tlb_invalidations; }
}
