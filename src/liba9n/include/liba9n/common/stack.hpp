#ifndef LIBA9N_COMMON_STACK_HPP
#define LIBA9N_COMMON_STACK_HPP

#include <liba9n/libcxx/type_traits>
#include <liba9n/libcxx/utility>
#include <liba9n/option/option.hpp>

namespace liba9n
{
    template<typename T, size_t Size>
    class stack
    {
      private:
        T      elements[Size];
        size_t current { Size };

      public:
        template<typename U>
            requires(liba9n::std::is_convertible_v<U, T>)
        void push(U &&value)
        {
            // decrement first
            elements[--current] = liba9n::std::forward<U>(value);
        }

        T &pop() &
        {
            // pop first
            return elements[current++];
        };

        T &&pop() &&
        {
            // 上記と同様
            return liba9n::std::move(elements[current++]);
        }
    };

    template<typename T, size_t Size>
    // Size : power of 2
        requires(
            (Size & (Size - 1)) == 0 &&             // Ensure Size is power of 2
            liba9n::std::is_copy_constructible_v<T> // T must be copy constructible
        )
    class queue
    {
      private:
        T      elements[Size];  // Storage array for queue
        size_t read_index  = 0; // Read index
        size_t write_index = 0; // Write index

        // Helper to calculate current size of the queue
        size_t current_size() const
        {
            return write_index - read_index;
        }

      public:
        template<typename U>
            requires(liba9n::std::is_convertible_v<U, T>) // Ensure U can be converted to T
        void enqueue(U &&value)
        {
            if (current_size() == Size) // Queue is full
            {
                return; // Optionally handle this case (e.g., throw, block, etc.)
            }

            elements[write_index & (Size - 1)] = liba9n::std::forward<U>(value); // Insert value
            ++write_index; // Increment write index after insertion
        }

        liba9n::option<T> dequeue()
        {
            if (write_index == read_index) // Queue is empty
            {
                return {}; // Return empty option if no elements to dequeue
            }

            return elements[read_index++ % (Size - 1)]; // Return element and increment read index
        }
    };
}

#endif
