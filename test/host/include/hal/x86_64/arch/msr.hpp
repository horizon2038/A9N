#pragma once
#include <kernel/types.hpp>

namespace a9n::hal::x86_64
{
    namespace msr { inline constexpr uint32_t PAT = 0x277; }
    inline a9n::word _read_msr(uint32_t) { return 0x0106; }
    inline void _write_msr(uint32_t, a9n::word) {}
}
