#ifndef LIBA9N_BIT_HPP
#define LIBA9N_BIT_HPP

#include <kernel/types.hpp>

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
    template<a9n::word BitNumber>
    inline constexpr auto word_bit_flag = calculate_bit_flag<a9n::word, BitNumber>();

    template<a9n::word LowBit, a9n::word HighBit>
    inline constexpr auto word_bit_mask = calculate_bit_mask<a9n::word, LowBit, HighBit>();
}

#endif
