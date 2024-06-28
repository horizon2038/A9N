#ifndef LIBCXX_INVOKE_RESULT_HPP
#define LIBCXX_INVOKE_RESULT_HPP

#include <library/libcxx/__functional/reference_wrapper.hpp>
#include <library/libcxx/__type_traits/bool_constant.hpp>
#include <library/libcxx/__type_traits/decay.hpp>
#include <library/libcxx/__type_traits/enable_if.hpp>
#include <library/libcxx/__type_traits/is_base_of.hpp>
#include <library/libcxx/__type_traits/remove_reference.hpp>
#include <library/libcxx/__utility/forward.hpp>
#include <library/libcxx/__utility/declval.hpp>

namespace library::std
{
    namespace
    {
        template<typename T>
        struct is_reference_wrapper : false_type
        {
        };

        template<typename U>
        struct is_reference_wrapper<reference_wrapper<U>> : true_type
        {
        };

        template<typename T>
        inline constexpr bool is_reference_wrapper_v
            = is_reference_wrapper<T>::value;

        // normal function calling
        template<typename T>
        struct invoke_impl
        {
            template<typename Function, typename... Args>
            static auto call(Function &&function, Args &&...args)
                -> decltype(forward<Function>(function)(forward<Args>(args)...)
                );
        };

        // member function calling
        template<typename ClassType, typename MemberType>
        struct invoke_impl<MemberType ClassType::*>
        {
            template<typename T, typename TDecay = decay_t<T>>
                requires(is_base_of_v<ClassType, TDecay>)
            static auto get(T &&t) -> T &&;

            template<typename T, typename TDecay = decay_t<T>>
                requires(is_reference_wrapper_v<TDecay>)
            static auto get(T &&t) -> decltype(t.get());

            template<typename T, typename TDecay = decay_t<T>>
                requires(!is_base_of_v<ClassType, TDecay> && !is_reference_wrapper_v<TDecay>)
            static auto get(T &&t) -> decltype(*forward<T>(t));

            template<typename T, typename... Args, typename MemberTypeOther>
                requires(is_function_v<MemberTypeOther>)
            static auto call(
                MemberTypeOther ClassType::*member_function_pointer,
                T                         &&t,
                Args &&...args
            )
                -> decltype((invoke_impl::get(forward<T>(t)).*member_function_pointer)(
                    forward<Args>(args)...
                ));

            template<typename T>
            static auto call(
                MemberType ClassType::*member_function_pointer,
                T                    &&t
            ) -> decltype(invoke_impl::get(forward<T>(t)).*member_function_pointer);
        };

        template<
            typename Function,
            typename... Args,
            typename FunctionDecay = decay_t<Function>>
        auto run_invoke(Function &&function, Args &&...args)
            -> decltype(invoke_impl<FunctionDecay>::call(
                forward<Function>(function),
                forward<Args>(args)...
            ));

        template<typename T, typename, typename...>
        struct invoke_result_definition
        {
        };

        template<typename Function, typename... Args>
        struct invoke_result_definition<
            decltype(void(run_invoke(declval<Function>(), declval<Args>()...))),
            Function,
            Args...>
        {
            using type
                = decltype(run_invoke(declval<Function>(), declval<Args>()...));
        };

    }

    template<typename Function, typename... ArgTypes>
    struct invoke_result : invoke_result_definition<void, Function, ArgTypes...>
    {
    };

    template<typename Function, typename... ArgTypes>
    using invoke_result_t = typename invoke_result<Function, ArgTypes...>::type;
}

#endif
