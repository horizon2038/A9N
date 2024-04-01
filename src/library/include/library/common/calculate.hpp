#ifndef CALCULATE_HPP
#define CALCULATE_HPP

#include <library/common/types.hpp>

namespace library::common
{
    inline word round_up_power_of_2(word value)
    {
        word power = 1;
        while (power < value)
        {
            power <<= 1;
        }
        return power;
    }

    inline word align_value(word value, word base)
    {
        if (base == 0)
        {
            return 0;
        }

        word aligned_value = (value + base - 1) / base * base;
        return aligned_value;
    }

    inline word align_down_power_of_2(word value, word base)
    {
        value &= ~(base - 1);
        return value;
    }

    inline word aligne_up_power_of_2(word value, word base)
    {
        value += (base - 1);
        return align_down_power_of_2(value, base);
    }
}

#endif
