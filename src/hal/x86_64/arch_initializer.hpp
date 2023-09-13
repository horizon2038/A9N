#ifndef X86_64_ARCHITECTURE_INITIALIZER
#define X86_64_ARCHITECTURE_INITIALIZER

#include <interface/arch_initializer.hpp>
#include "segment_configurator.hpp"

namespace hal::x86_64
{
    class arch_initializer final : public hal::interface::arch_initializer
    {
        public:
            arch_initializer(segment_configurator &injected_segment_configurator);
            ~arch_initializer();
            void init_architecture() override;

        private:
            segment_configurator &_segment_configurator; 
    };
}

#endif
