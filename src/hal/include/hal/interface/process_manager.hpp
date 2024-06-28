#ifndef HAL_CONTEXT_SWITCH_HPP
#define HAL_CONTEXT_SWITCH_HPP

#include <kernel/process/process.hpp>

#include <library/common/types.hpp>

namespace a9n::hal
{
    class process_manager
    {
      public:
        virtual void switch_context(
            a9n::kernel::process *preview_process,
            a9n::kernel::process *next_process
        ) = 0;
        virtual void create_process(
            a9n::kernel::process        *target_process,
            a9n::virtual_address entry_point_address
        )                                                            = 0;
        virtual void delete_process(a9n::kernel::process *target_process) = 0;
    };
}

#endif
