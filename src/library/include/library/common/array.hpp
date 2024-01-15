#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <library/common/types.hpp>

namespace library::common
{
    template<typename T, common::word size>
    class bounded_array
    {
      public:
        const T get_element(common::word index)
        {
            if (index >= size)
            {
                return 0;
            }
            return elements[index];
        }

        void set_element(common::word index, T data)
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
