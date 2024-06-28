#ifndef X86_64_CONTEXT_SWITCH_HPP
#define X86_64_CONTEXT_SWITCH_HPP

#include <liba9n/common/types.hpp>
#include <hal/interface/process_manager.hpp>

#include <hal/x86_64/arch/segment_configurator.hpp>
#include <hal/x86_64/memory/paging.hpp>

namespace a9n::hal::x86_64
{
    class process_manager final : public a9n::hal::process_manager
    {
      public:
        process_manager();
        ~process_manager();
        void switch_context(
            a9n::kernel::process *preview_process,
            a9n::kernel::process *next_process
        ) override;
        void create_process(
            a9n::kernel::process *target_process,
            a9n::virtual_address  entry_point_address
        ) override;
        void delete_process(a9n::kernel::process *target_process) override;

      private:
        segment_configurator _segment_configurator;
    };
}

#endif
