#include <hal/interface/init.hpp>

namespace a9n::hal
{
    // hal-level initial allocator
    liba9n::linear_allocator<a9n::PAGE_SIZE * 16> init_allocator {};

    hal_result create_init_interrupt_port(liba9n::not_null<a9n::kernel::capability_component> node)
    {
        return {};
    }
}
