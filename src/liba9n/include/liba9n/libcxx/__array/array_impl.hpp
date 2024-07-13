#ifndef LIBA9N_LIBCXX_ARRAY_IMPL_HPP
#define LIBA9N_LIBCXX_ARRAY_IMPL_HPP

#include <liba9n/libcxx/__array/array_common.hpp>

#include <liba9n/libcxx/__utility/move.hpp>

namespace liba9n::std
{
    template<typename T, auto Size>
    class array
    {
      private:
        // common data sections
        T elements[Size];

      public:
        using reference       = T &;
        using const_reference = const T &;

        // common type sections
        using size_type = decltype(Size);

        // iterator definition
        using iterator       = T *;
        using const_iterator = const T *;

        // TODO: implement reverse iterator
        // using reverse_iterator       = ;
        // using const_reverse_iterator = ;

        // using difference_type = ptrdiff_t;
        using pointer       = T *;
        using const_pointer = const T *;
        using value_type    = T;

        // common functions
        constexpr T *get_elements_first()
        {
            return elements;
        }

        constexpr const T *get_elements_first() const
        {
            return elements;
        }

        constexpr T *get_elements_end()
        {
            return elements + (Size - 1);
        }

        constexpr const T *get_elements_end() const
        {
            return elements + (Size - 1);
        }

        // element access
        constexpr T &at(size_type index)
        {
            if (index >= Size) [[unlikely]]
            {
                return;
            }

            return elements[index];
        }

        constexpr const T &at(size_type index) const
        {
            if (index >= Size) [[unlikely]]
            {
                return;
            }

            return elements[index];
        }

        constexpr T &front()
        {
            return *get_elements_first();
        }

        constexpr const T &front() const
        {
            return *get_elements_first();
        }

        constexpr T &back()
        {
            return *get_elements_end();
        }

        constexpr const T &back() const
        {
            return *get_elements_end();
        }

        constexpr T *data()
        {
            return static_cast<T *>(elements);
        }

        constexpr const T *data() const
        {
            return static_cast<T *>(elements);
        }

        // capacity
        constexpr size_type empty() const
        {
            // true only if array<T, 0>
            return false;
        }

        constexpr size_type size() const
        {
            return Size;
        }

        // for compatibility with another container type
        constexpr size_type max_size() const
        {
            return Size;
        }

        // operations
        constexpr void fill(const T &value)
        {
            // TODO: replace to liba9n::std::fill
            for (size_type i = 0; i < Size; i++)
            {
                elements[i] = value;
            }
        }

        constexpr void swap(array &other)
        {
            // TODO: replace to liba9n::std::swap_ranges
            using forward_iterator_1 = decltype(begin());
            using forward_iterator_2 = decltype(other.begin());

            forward_iterator_1 this_begin  = begin();
            forward_iterator_1 this_end    = end();
            forward_iterator_2 other_begin = other.begin();

            while (this_begin != this_end)
            {
                T temp       = liba9n::std::move(*this_begin);
                *this_begin  = liba9n::std::move(*other_begin);
                *other_begin = liba9n::std::move(temp);

                this_begin++;
                other_begin++;
            }
        }

        // tuple interfaces
        // not implemented :
        // - tuple_element
        // - tuple_size

        // operators
        constexpr T &operator[](size_type index)
        {
            static_assert(
                index < Size,
                "out of bounds access in liba9n::std::array"
            );

            return elements[index];
        }

        constexpr const T &operator[](size_type index) const
        {
            static_assert(
                index < Size,
                "out of bounds access in liba9n::std::array"
            );

            return elements[index];
        }

        // iterators
        constexpr iterator begin()
        {
            return iterator(get_elements_first());
        }

        constexpr const_iterator begin() const
        {
            return const_iterator(get_elements_first());
        }

        constexpr iterator end()
        {
            return iterator(get_elements_end());
        }

        constexpr const_iterator end() const
        {
            return iterator(get_elements_end());
        }

        // only const
        constexpr const_iterator cbegin() const
        {
            return begin();
        }

        constexpr const_iterator cend() const
        {
            return end();
        }

        /*
        constexpr reverse_iterator rbegin()
        {
        }

        constexpr const_reverse_iterator rbegin() const
        {
        }

        constexpr const_reverse_iterator crbegin() const
        {
        }

        constexpr const_reverse_iterator crend() const
        {
        }
        */
    };

    template<typename T, auto Size>
    inline constexpr bool
        operator==(const array<T, Size> &lhs, const array<T, Size> &rhs)
    {
        return !(lhs == rhs);
    }

    template<typename T, auto Size>
    inline constexpr bool
        operator!=(const array<T, Size> &lhs, const array<T, Size> &rhs)
    {
        return !(lhs == rhs);
    }

}

#endif
