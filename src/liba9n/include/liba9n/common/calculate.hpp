#ifndef CALCULATE_HPP
#define CALCULATE_HPP

#include <kernel/types.hpp>

namespace liba9n
{
    inline constexpr a9n::word round_up_power_of_2(a9n::word value)
    {
        [[unlikely]] if (!value)
        {
            return 1;
        }

        if ((value & (value - 1)) == 0)
        {
            return value;
        }

        a9n::word leading_zero   = __builtin_clzll(value);
        a9n::word highest_bit    = a9n::WORD_BITS - 1 - leading_zero;

        a9n::word power          = 1;
        power                  <<= (highest_bit + 1);

        return power;
    }

    inline constexpr a9n::word round_down_power_of_2(a9n::word value)
    {
        [[unlikely]] if (!value)
        {
            return 0;
        }

        a9n::word leading_zero   = __builtin_clzll(value);
        a9n::word highest_bit    = a9n::WORD_BITS - 1 - leading_zero;

        a9n::word power          = 1;
        power                  <<= highest_bit;

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

    inline a9n::word align_up_power_of_2(a9n::word value, a9n::word base)
    {
        value += (base - 1);
        return align_down_power_of_2(value, base);
    }

    inline constexpr a9n::word calculate_radix(a9n::word power_of_2)
    {
        [[unlikely]] if (!power_of_2)
        {
            return 0;
        }

        a9n::word radix = 0;
        while (power_of_2 >>= 1)
        {
            radix++;
        }

        return radix;
    }

    inline constexpr a9n::word calculate_radix_ceil(a9n::word value)
    {
        [[unlikely]] if (!value)
        {
            return 0;
        }

        a9n::word rounded_value = round_up_power_of_2(value);

        return calculate_radix(rounded_value);
    }

    inline constexpr a9n::word calculate_radix_floor(a9n::word value)
    {
        [[unlikely]] if (!value)
        {
            return 0;
        }

        a9n::word rounded_value = round_down_power_of_2(value);

        return calculate_radix(rounded_value);
    }
}

#endif
