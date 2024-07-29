#ifndef x86_64_SYSCALL_HPP
#define x86_64_SYSCALL_HPP

#include <stdint.h>

namespace a9n::hal::x86_64
{
    namespace MSR
    {
        static constexpr uint32_t EFER   = 0xC0000080;
        static constexpr uint32_t STAR   = 0xc0000082;
        static constexpr uint32_t LSTAR  = 0xc0000083;
        static constexpr uint32_t CSTAR  = 0xc0000084;
        static constexpr uint32_t SFMASK = 0xc0000084;
    }

    class syscall
    {
      public:
        void init_syscall();
        void handle_syscall();
    };
}

#endif
