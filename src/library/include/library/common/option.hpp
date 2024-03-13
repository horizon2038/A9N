#ifndef LIBRARY_OPTION_HPP
#define LIBRARY_OPTION_HPP

#include <library/libcxx/utility>
#include <library/libcxx/type_traits>

#include <kernel/utility/logger.hpp>

namespace library::common
{
    struct option_none
    {
    };

    template<typename T>
    class option
    {
      public:
        option() : has_value_flag(false)
        {
        }

        option([[maybe_unused]] option_none n) : has_value_flag(false)
        {
        }

        option(T &&t) : has_value_flag(true), value(library::std::forward<T>(t))
        {
        }

        option(const T &t) : has_value_flag(true), value(t)
        {
        }

        option(const option &other) : has_value_flag(other.has_value_flag)
        {
            if (other.has_value_flag)
            {
                value = other.value;
            }
        }

        option(option &&other) : has_value_flag(other.has_value_flag)
        {
            if (other.has_value_flag)
            {
                value = library::std::move(other.value);
            }
        }

        const T &unwrap() const
        {
            return value;
        }

        T &unwrap()
        {
            return value;
        }

        bool has_value()
        {
            return has_value_flag;
        }

        // TODO: add monadic operations :
        // e.g. and_then(), transform(), or_else()

      private:
        bool has_value_flag;
        T value;
    };

    template<typename T>
    auto make_option_some(T &&t) -> option<library::std::remove_reference_t<T>>
    {
        return library::std::forward<T>(t);
    }

    inline auto make_option_none() -> decltype(auto)
    {
        return option_none();
    }

}

#endif
