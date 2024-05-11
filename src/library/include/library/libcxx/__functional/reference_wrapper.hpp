#ifndef LIBCXX_REFERENCE_WRAPPER_HPP
#define LIBCXX_REFERENCE_WRAPPER_HPP

#include <library/libcxx/__memory/addressof.hpp>
#include <library/libcxx/__utility/forward.hpp>

namespace library::std
{
    template<typename T>
    class reference_wrapper
    {
      public:
        explicit reference_wrapper(T &t) noexcept : pointer(addressof(t))
        {
        }

        reference_wrapper(reference_wrapper const &other) noexcept
            : pointer(other.pointer)
        {
        }

        reference_wrapper &operator=(reference_wrapper const &other) noexcept
        {
            pointer = other.pointer;
            return *this;
        }

        T &get() const noexcept
        {
            return *pointer;
        }

        operator T &() const noexcept
        {
            return *pointer;
        }

        template<typename... A>
        auto operator()(A &&...args) const -> decltype(auto)
        {
            return (*pointer)(forward<A>(args)...);
        }

      private:
        T *pointer;
    };

    template<typename T>
    constexpr reference_wrapper<T> ref(T &t) noexcept
    {
        return reference_wrapper<T>(t);
    }

    template<typename T>
    constexpr reference_wrapper<T> cref(const T &t) noexcept
    {
        return reference_wrapper<const T>(t);
    }
}

#endif
