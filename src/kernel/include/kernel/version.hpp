#include <liba9n/common/version.hpp>

#ifndef KERNEL_VERSION
#define KERNEL_VERSION "0.1.0-unknown+00000000-UNKNOWN"
#endif

namespace a9n::kernel
{
    inline constexpr const char KERNEL_VERSION_STRING[] = KERNEL_VERSION;
}
