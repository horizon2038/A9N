#pragma once
#define current_user_page_table unused_privileged_current_user_page_table
#include "../../../../../../src/hal/aarch64/include/hal/aarch64/memory/paging.hpp"
#undef current_user_page_table
#include <test_machine.hpp>

namespace a9n::hal::aarch64
{
    inline a9n::physical_address current_user_page_table() { return test_machine::active_root; }
}
