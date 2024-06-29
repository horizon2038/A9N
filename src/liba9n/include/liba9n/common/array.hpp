#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <kernel/types.hpp>

#include <liba9n/option/option.hpp>

namespace liba9n::common
{
    template<typename T, a9n::word size>
    class bounded_array
    {
      public:
        const T get_element(a9n::word index) const
        {
            if (index >= size)
            {
                return 0;
            }
            return elements[index];
        }

        void set_element(a9n::word index, T data)
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
        liba9n::option<T> get_element(size_type index)
        {
            if (index >= Size) [[unlikely]]
            {
                return {};
            }

            return elements[index];
        }

        void set_element(size_type index, const T &value)
        {
            if (index >= Size) [[unlikely]]
            {
                return;
            }

            elements[index] = value;
        }

        void fill(const T &data)
        {
            for (size_type i = 0; i < Size; i++)
            {
                elements[i] = data;
            }
        }
    };
}

#endif
