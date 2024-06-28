#ifndef CALCULATE_HPP
#define CALCULATE_HPP

#include <library/common/types.hpp>

namespace liba9n::common
{
    inline a9n::word round_up_power_of_2(a9n::word value)
    {
        a9n::word power = 1;
        while (power < value)
        {
            power <<= 1;
        }
        return power;
    }

    inline a9n::word align_value(a9n::word value, a9n::word base)
    {
        if (base == 0)
        {
            return 0;
        }

        a9n::word aligned_value = (value + base - 1) / base * base;
        return aligned_value;
    }

    inline a9n::word align_down_power_of_2(a9n::word value, a9n::word base)
    {
        value &= ~(base - 1);
        return value;
    }

    inline a9n::word aligne_up_power_of_2(a9n::word value, a9n::word base)
    {
        value += (base - 1);
        return align_down_power_of_2(value, base);
    }
}

#endif
