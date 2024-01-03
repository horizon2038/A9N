#ifndef X86_64_ARCH_INITIALIZER_HPP
#define X86_64_ARCH_INITIALIZER_HPP

#include <hal/interface/arch_initializer.hpp>

namespace hal::x86_64
{
    class arch_initializer final : public hal::interface::arch_initializer
    {
      public:
        arch_initializer();
        ~arch_initializer();

        void init_architecture(common::word arch_info[]) override;

      private:
    };
}

#endif
