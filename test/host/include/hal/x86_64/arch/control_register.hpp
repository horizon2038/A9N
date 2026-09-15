#pragma once
#include <test_machine.hpp>

namespace a9n::hal::x86_64
{
    inline a9n::word _read_cr3() { return test_machine::active_root; }
}
