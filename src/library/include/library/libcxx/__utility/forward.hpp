#ifndef LIBCXX_FORWARD_HPP
#define LIBCXX_FORWARD_HPP

namespace std
{
    template<typename T>
    constexpr decltype(auto) forward(T &&t) noexcept
    {
        return static_cast<decltype(t)>(t);
    }
}

#endif
