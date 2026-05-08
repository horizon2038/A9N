#ifndef LIBA9N_ARRAY_HPP
#define LIBA9N_ARRAY_HPP

#include <liba9n/option/option.hpp>
#include <stdint.h>

namespace liba9n::common
{
    template<typename T, uintmax_t size>
    class bounded_array
    {
      public:
        const T get_element(uintmax_t index) const
        {
            if (index >= size)
            {
                return 0;
            }
            return elements[index];
        }

        void set_element(uintmax_t index, T data)
        {
            if (index >= size)
            {
                return;
            }
            elements[index] = data;
        }

        void fill(T data)
        {
            for (auto i = 0; i < size; i++)
            {
                elements[i] = data;
            }
        }

      private:
        T elements[size];
    };

    template<typename T, auto Size>
    class safe_array
    {
        using size_type = decltype(Size);

      private:
        T elements[Size];

      public:
        template<size_type Index>
            requires(0 <= Index && Index < Size)
        constexpr T &get()
        {
            return elements[Index];
        }

        template<size_type Index>
            requires(0 <= Index && Index < Size)
        constexpr const T &get() const
        {
            return elements[Index];
        }

        template<size_type Index>
            requires(0 <= Index && Index < Size)
        constexpr void set(const T &value)
        {
            elements[Index] = value;
        }

        void fill(const T &data)
        {
            for (size_type i = 0; i < Size; i++)
            {
                elements[i] = data;
            }
        }

        // iterator sections
        T *begin() &
        {
            return &(elements[0]);
        }

        const T *begin() const &
        {
            return &(elements[0]);
        };

        T *end() &
        {
            return &(elements[Size - 1]);
        }

        const T *end() const &
        {
            return &(elements[Size - 1]);
        }
    };
}

#endif
