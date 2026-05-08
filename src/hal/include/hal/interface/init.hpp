#ifndef A9N_HAL_INIT_HPP
#define A9N_HAL_INIT_HPP

#include <kernel/capability/capability_component.hpp>
#include <liba9n/common/allocator.hpp>
#include <liba9n/common/not_null.hpp>
#include <liba9n/result/result.hpp>

#include <hal/hal_result.hpp>

namespace a9n::hal
{
    hal_result create_init_interrupt_port(liba9n::not_null<a9n::kernel::capability_component> node);

    struct hal_info
    {
        a9n::word hal_version;
        a9n::word boot_info_page_count;
        a9n::word interrupt_port_count_max;
    };
}

#endif
