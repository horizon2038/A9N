#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <kernel/types.hpp>

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
}

#endif
