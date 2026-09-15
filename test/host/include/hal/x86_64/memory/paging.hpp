#pragma once
#include <hal/x86_64/arch/arch_types.hpp>
#include <kernel/utility/logger.hpp>
// Keep production PTEs and address calculations; replace privileged effects only.
// Darwin's uint64_t and uintptr_t are different integer typedefs. Match the
// kernel target's typedefs for its existing integer reinterpret_cast in page.
#define uint64_t a9n::word
#define _flush_tlb unused_privileged_flush_tlb
#define _invalidate_page unused_privileged_invalidate_page
#include "../../../../../../src/hal/x86_64/include/hal/x86_64/memory/paging.hpp"
#undef uint64_t
#undef _flush_tlb
#undef _invalidate_page
#include <test_machine.hpp>

namespace a9n::hal::x86_64
{
    inline void _flush_tlb() { ++test_machine::tlb_invalidations; }
    inline void _invalidate_page(a9n::virtual_address) { ++test_machine::tlb_invalidations; }
}
