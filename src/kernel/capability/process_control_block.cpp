#include <kernel/capability/process_control_block.hpp>

#include <hal/interface/process_manager.hpp>

namespace a9n::kernel
{
    process_control_block::process_control_block()
    {
        // init hardware-specific contexts
        hal::init_hardware_context(process_core.registers);
    }
}
