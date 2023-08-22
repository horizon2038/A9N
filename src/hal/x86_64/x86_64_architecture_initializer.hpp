#ifndef X86_64_ARCHITECTURE_INITIALIZER
#define X86_64_ARCHITECTURE_INITIALIZER

#include <interface/architecture_initializer.hpp>
#include "segment_configurator.hpp"

namespace hal::x86_64
{
    class architecture_initializer final : public hal::interface::architecture_initializer
    {
        public:
            architecture_initializer(segment_configurator &injected_segment_configurator);
            ~architecture_initializer();
            void init_architecture() override;

        private:
            segment_configurator &_segment_configurator; 
    };
}

#endif
