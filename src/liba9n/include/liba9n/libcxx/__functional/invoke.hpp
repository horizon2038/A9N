#ifndef LIBCXX_INVOKE_HPP
#define LIBCXX_INVOKE_HPP

#include <liba9n/libcxx/__functional/reference_wrapper.hpp>
#include <liba9n/libcxx/__type_traits/invoke_result.hpp>
#include <liba9n/libcxx/__type_traits/is_base_of.hpp>
#include <liba9n/libcxx/__type_traits/is_function.hpp>
#include <liba9n/libcxx/__type_traits/is_member_pointer.hpp>
#include <liba9n/libcxx/__type_traits/remove_cvref.hpp>

namespace liba9n::std
{
    namespace detail
    {
        template<typename ClassType, typename Pointer, typename Object, typename... Args>
        constexpr decltype(auto
        ) invoke_member_pointer(Pointer ClassType::*member, Object &&object, Args &&...args)
        {
            constexpr bool is_member_function = is_function_v<Pointer>;
            constexpr bool is_wrapped         = is_reference_wrapper_v<remove_cvref_t<Object>>;
            constexpr bool is_derived_object
                = is_same_v<ClassType, remove_cvref_t<Object>>
               || is_base_of_v<ClassType, remove_cvref_t<Object>>;

            // for normal class member function
            if constexpr (is_member_function)
            {
                if constexpr (is_derived_object)
                {
                    return (forward<Object>(object).*member)(forward<Args>(args)...);
                }

                else if constexpr (is_wrapped)
                {
                    return (object.get.*member)(forward<Args>(args)...);
                }

                else
                {
                    return ((*forward<Object>(object)).*member)(forward<Args>(args)...);
                }
            }

            // data member (not member function)
            // std::invoke can *invoke* data members.
            else
            {
                if constexpr (is_derived_object)
                {
                    return forward<Object>(object).*member;
                }

                else if constexpr (is_wrapped)
                {
                    return object.get().*member;
                }

                else
                {
                    return (*forward<Object>(object)).*member;
                }
            }
        }
    }

    template<typename Function, typename... Args>
    constexpr invoke_result_t<Function, Args...> invoke(Function &&function, Args &&...args) noexcept
    {
        if constexpr (is_member_pointer_v<remove_cvref_t<Function>>)
        {
            return detail::invoke_member_pointer(function, forward<Args>(args)...);
        }

        else
        {
            return forward<Function>(function)(forward<Args>(args)...);
        }
    }
}

#endif
