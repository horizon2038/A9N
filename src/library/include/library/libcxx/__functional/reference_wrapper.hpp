#ifndef LIBCXX_REFERENCE_WRAPPER_HPP
#define LIBCXX_REFERENCE_WRAPPER_HPP

namespace library::std
{
    // TODO : fix this
    template<typename T>
    class reference_wrapper
    {
      public:
        constexpr reference_wrapper(T &t) noexcept : ref_(t) {};

        constexpr reference_wrapper(const reference_wrapper &other) noexcept
            : ref_(other.ref_)
        {
        }

        reference_wrapper &operator=(const reference_wrapper &other) noexcept
        {
            ref_ = other.ref_;
            return *this;
        }

        T &operator*() noexcept
        {
            return ref_;
        }

        const T &operator*() const noexcept
        {
            return ref_;
        }

        T &get() noexcept
        {
            return ref_;
        }

        const T &get() const noexcept
        {
            return ref_;
        }

      private:
        T &ref_;
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
