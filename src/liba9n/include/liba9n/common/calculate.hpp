#ifndef CALCULATE_HPP
#define CALCULATE_HPP

#include <stdint.h>

namespace liba9n
{
    inline constexpr uintmax_t BYTE_BITS = 8;
    inline constexpr uintmax_t WORD_BITS = sizeof(uintmax_t) * BYTE_BITS;

    // todo: rename to `round_up_power_of_two_ceil`
    inline constexpr uintmax_t round_up_power_of_2(uintmax_t value)
    {
        [[unlikely]] if (!value)
        {
            return 1;
        }

        if ((value & (value - 1)) == 0)
        {
            return value;
        }

        uintmax_t leading_zero   = __builtin_clzll(value);
        uintmax_t highest_bit    = WORD_BITS - 1 - leading_zero;

        uintmax_t power          = 1;
        power                  <<= (highest_bit + 1);

        return power;
    }

    // todo: rename to `round_up_power_of_two_floor`
    inline constexpr uintmax_t round_down_power_of_2(uintmax_t value)
    {
        [[unlikely]] if (!value)
        {
            return 0;
        }

        uintmax_t leading_zero   = __builtin_clzll(value);
        uintmax_t highest_bit    = WORD_BITS - 1 - leading_zero;

        uintmax_t power          = 1;
        power                  <<= highest_bit;

        return power;
    }

    // TODO: rename to `align_value_ceil`
    inline constexpr uintmax_t align_value(uintmax_t value, uintmax_t base)
    {
        if (base == 0)
        {
            return 0;
        }

        return (value + base - 1) / base * base;
    }

    inline constexpr uintmax_t align_value_floor(uintmax_t value, uintmax_t base)
    {
        if (base == 0)
        {
            return 0;
        }

        return (value / base) * base;
    }

    inline constexpr uintmax_t calculate_radix(uintmax_t power_of_2)
    {
        [[unlikely]] if (!power_of_2)
        {
            return 0;
        }

        uintmax_t radix = 0;
        while (power_of_2 >>= 1)
        {
            radix++;
        }

        return radix;
    }

    inline constexpr uintmax_t calculate_radix_ceil(uintmax_t value)
    {
        [[unlikely]] if (!value)
        {
            return 0;
        }

        uintmax_t rounded_value = round_up_power_of_2(value);

        return calculate_radix(rounded_value);
    }

    inline constexpr uintmax_t calculate_radix_floor(uintmax_t value)
    {
        [[unlikely]] if (!value)
        {
            return 0;
        }

        uintmax_t rounded_value = round_down_power_of_2(value);

        return calculate_radix(rounded_value);
    }
}

#endif
