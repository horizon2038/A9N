#ifndef CALCULATE_HPP
#define CALCULATE_HPP

#include <library/common/types.hpp>

namespace library::common
{
    inline word round_up_power_of_two(word value)
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
        word aligned_value = (value + base - 1) / base * base;
        return aligned_value;
    }
}

#endif
