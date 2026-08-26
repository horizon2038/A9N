#ifndef A9N_KERNEL_CONFIG_HPP
#define A9N_KERNEL_CONFIG_HPP

namespace a9n::kernel
{
#ifdef A9N_CONFIG_ENABLE_SMP
    inline constexpr bool SMP_ENABLED = true;
#else
    inline constexpr bool SMP_ENABLED = false;
#endif
}

#endif
