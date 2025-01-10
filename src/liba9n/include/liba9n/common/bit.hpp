#ifndef LIBA9N_BIT_HPP
#define LIBA9N_BIT_HPP

#include <stdint.h>

namespace liba9n
{
    template<typename Base, Base BitNumber>
    inline constexpr auto calculate_bit_flag(void)
    {
        return static_cast<Base>(1) << BitNumber;
    }

    template<typename Base, Base BitNumber>
    inline constexpr auto bit_flag = calculate_bit_flag<Base, BitNumber>();

    template<typename Base, Base LowBit, Base HighBit>
        requires(LowBit < HighBit)
    inline constexpr auto calculate_bit_mask(void)
    {
        constexpr auto mask_width = HighBit - LowBit + 1;
        constexpr auto mask       = calculate_bit_flag<Base, mask_width>() - 1;

        return mask << LowBit;
    }

    template<typename Base, Base LowBit, Base HighBit>
    inline constexpr auto bit_mask = calculate_bit_mask<Base, LowBit, HighBit>();

    // word specialization
    template<uintmax_t BitNumber>
    inline constexpr auto word_bit_flag = calculate_bit_flag<uintmax_t, BitNumber>();

    template<uintmax_t LowBit, uintmax_t HighBit>
    inline constexpr auto word_bit_mask = calculate_bit_mask<uintmax_t, LowBit, HighBit>();
}

#endif
