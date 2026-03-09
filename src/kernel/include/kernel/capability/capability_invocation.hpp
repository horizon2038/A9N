#ifndef A9N_KERNEL_CAPABILITY_INVOCATION_HPP
#define A9N_KERNEL_CAPABILITY_INVOCATION_HPP

#include <hal/interface/process_manager.hpp>
#include <kernel/capability/capability_result.hpp>
#include <kernel/types.hpp>

namespace a9n::kernel
{
    namespace detail
    {
        inline constexpr capability_error
            convert_hal_to_capability_error([[maybe_unused]] a9n::hal::hal_error e)
        {
            return capability_error::FATAL;
        }

        template<a9n::word... MESSAGE_REGISTER_INDICES>
        struct message_register_reader;

        template<>
        struct message_register_reader<>
        {
            template<typename Function>
            static auto run([[maybe_unused]] process &owner, Function &&function) -> capability_result
            {
                return function();
            }
        };

        template<a9n::word HEAD, a9n::word... TAIL>
        struct message_register_reader<HEAD, TAIL...>
        {
            template<typename Function>
            static auto run(process &owner, Function &&function) -> capability_result
            {
                return a9n::hal::get_message_register(owner, HEAD)
                    .transform_error(convert_hal_to_capability_error)
                    .and_then(
                        [&](a9n::word value) -> capability_result
                        {
                            return message_register_reader<TAIL...>::run(
                                owner,
                                [&](auto &&...tail_values) -> capability_result
                                {
                                    return function(value, tail_values...);
                                }
                            );
                        }
                    );
            }
        };

        template<a9n::word... MESSAGE_REGISTER_INDICES>
        struct message_register_writer;

        template<>
        struct message_register_writer<>
        {
            static auto run([[maybe_unused]] process &owner) -> capability_result
            {
                return {};
            }
        };

        template<a9n::word HEAD, a9n::word... TAIL>
        struct message_register_writer<HEAD, TAIL...>
        {
            template<typename... RestValues>
            static auto run(process &owner, a9n::word head_value, RestValues... rest_values)
                -> capability_result
            {
                return a9n::hal::configure_message_register(owner, HEAD, head_value)
                    .transform_error(convert_hal_to_capability_error)
                    .and_then(
                        [&](void) -> capability_result
                        {
                            return message_register_writer<TAIL...>::run(owner, rest_values...);
                        }
                    );
            }
        };
    }

    // how to use:
    // with_message_registers<MR0, MR1, MR2>(
    //   process,
    //   [](a9n::word mr0, a9n::word mr1, a9n::word mr2
    // ) -> capability_result { ... });
    template<a9n::word... INDICES, typename Function>
    auto with_message_registers(process &owner, Function &&function) -> capability_result
    {
        return detail::message_register_reader<INDICES...>::run(
            owner,
            liba9n::std::forward<Function>(function)
        );
    }

    // how to use:
    // write_message_registers<MR0, MR1, MR2>(
    //   process,
    //   value0, value1, value2
    // );
    template<a9n::word... INDICES, typename... Values>
        requires(sizeof...(INDICES) == sizeof...(Values))
    auto write_message_registers(process &owner, Values... values) -> capability_result
    {
        return detail::message_register_writer<INDICES...>::run(owner, values...);
    }
}

#endif
