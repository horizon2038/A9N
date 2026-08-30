#include <hal/interface/virtualize.hpp>

namespace a9n::hal
{
    hal_result try_init_virtual_cpu(a9n::kernel::virtual_cpu &)
    {
        return hal_error::UNSUPPORTED;
    }
}
