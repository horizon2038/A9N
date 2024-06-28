#ifndef X86_64_ARCH_INITIALIZER_HPP
#define X86_64_ARCH_INITIALIZER_HPP

#include <hal/interface/arch_initializer.hpp>

namespace a9n::hal::x86_64
{
    class arch_initializer final : public a9n::hal::arch_initializer
    {
      public:
        arch_initializer();
        ~arch_initializer();

        void init_architecture(a9n::word arch_info[]) override;

      private:
    };
}

#endif
