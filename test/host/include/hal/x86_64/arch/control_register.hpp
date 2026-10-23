#pragma once
#include <test_machine.hpp>

namespace a9n::hal::x86_64
{
    inline a9n::word _read_cr3() { return test_machine::active_root; }
    inline a9n::word _read_cr0() { return 0; }
    inline a9n::word _read_cr4() { return 0; }
    inline void _write_cr0(a9n::word) {}
    inline void _write_cr3(a9n::word value) { test_machine::active_root = value; }
    inline void _write_cr4(a9n::word) {}
    inline void _writeback_invalidate_caches() {}
    namespace cr0_flag
    {
        inline constexpr a9n::word CACHE_DISABLE = 1ULL << 30;
        inline constexpr a9n::word NOT_WRITE_THROUGH = 1ULL << 29;
    }
    namespace cr4_flag
    {
        inline constexpr a9n::word PCID = 1ULL << 17;
        inline constexpr a9n::word PAGE_GLOBAL = 1ULL << 7;
    }
}
