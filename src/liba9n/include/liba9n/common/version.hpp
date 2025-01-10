#ifndef LIBA9N_VERSION_HPP
#define LIBA9N_VERSION_HPP

#include <liba9n/option/option.hpp>

namespace liba9n
{
    struct value_with_current
    {
        value_with_current(uintmax_t target_value, const char *target_current)
            : value { target_value }
            , current { target_current }
        {
        }

        uintmax_t   value;
        const char *current;
    };

    template<char... Separators>
    inline liba9n::option<value_with_current> string_to_value(const char *string)
    {
        if (!string)
        {
            return {};
        }

        uintmax_t value      = 0;
        bool      has_digits = false;

        auto is_separator    = [&](char c) -> bool
        {
            return ((c == Separators) || ...);
        };

        while (*string != '\0' && !is_separator(*string))
        {
            if (*string >= '0' && *string <= '9')
            {
                has_digits      = true;
                uintmax_t digit = *string - '0';

                if (value > (UINTMAX_MAX - digit) / 10)
                {
                    return {};
                }

                value = value * 10 + digit;
            }
            else
            {
                return {};
            }

            string++;
        }

        if (!has_digits)
        {
            return {};
        }

        if (is_separator(*string))
        {
            string++;
        }

        return value_with_current(value, string);
    }

    class semantic_version
    {
      public:
        semantic_version(const char *version);
        semantic_version(
            uintmax_t   major,
            uintmax_t   minor,
            uintmax_t   patch,
            const char *pre_release     = "",
            const char *build_meta_data = ""
        );

        uintmax_t current_major(void) const
        {
            return major;
        }

        uintmax_t current_minor(void) const
        {
            return minor;
        }

        uintmax_t current_patch(void) const
        {
            return patch;
        }

        const char *current_pre_release(void) const
        {
            return pre_release;
        }

        const char *current_build_meta_data(void) const
        {
            return build_meta_data;
        }

      private:
        enum class state
        {
            UNINITIALIZED,
            MAJOR,
            MINOR,
            PATCH,
            PRE_RELEASE,
            BUILD_META_DATA,
            FINISHED,
        } current_state { state::UNINITIALIZED };

        uintmax_t major { 0 };
        uintmax_t minor { 0 };
        uintmax_t patch { 0 };

        // option field
        char pre_release[32];
        char build_meta_data[32];

        inline liba9n::option<const char *> parse_base(const char *base_version);
        liba9n::option<const char *>        parse_pre_release(const char *pre_release);
        liba9n::option<const char *>        parse_build_meta_data(const char *build_meta_data);
    };

    // example
    // 1.0.0
    // 1.0.0 -pre-alpha.0.1 +exp.test.debug

    inline semantic_version::semantic_version(const char *version)
    {
        parse_base(version)
            .and_then(
                [&, this](const char *remain) -> liba9n::option<const char *>
                {
                    return parse_pre_release(remain);
                }
            )
            .and_then(
                [&, this](const char *remain) -> liba9n::option<const char *>
                {
                    return parse_build_meta_data(remain);
                }
            );
    }

    inline semantic_version::semantic_version(
        uintmax_t   target_major,
        uintmax_t   target_minor,
        uintmax_t   target_patch,
        const char *target_pre_release,
        const char *target_build_meta_data
    )
    {
        major = target_major;
        minor = target_major;
        patch = target_patch;

        // copy pre_release
        // copy build_meta_data
    }

    inline liba9n::option<const char *> semantic_version::parse_base(const char *base_version)
    {
        if (current_state != state::UNINITIALIZED || !base_version)
        {
            return {};
        }

        current_state = state::MAJOR;

        while (*base_version)
        {
            switch (current_state)
            {
                case state::MAJOR :
                    if (auto result = string_to_value<'.'>(base_version))
                    {
                        major         = result.unwrap().value;
                        base_version  = result.unwrap().current;
                        current_state = state::MINOR;
                        break;
                    }
                    else
                    {
                        return {};
                    }

                case state::MINOR :
                    if (auto result = string_to_value<'.'>(base_version))
                    {
                        minor         = result.unwrap().value;
                        base_version  = result.unwrap().current;
                        current_state = state::PATCH;
                        break;
                    }
                    else
                    {
                        return {};
                    }

                case state::PATCH :
                    if (auto result = string_to_value<'-', '+'>(base_version))
                    {
                        patch        = result.unwrap().value;
                        base_version = result.unwrap().current;

                        switch (*(base_version - 1))
                        {
                            case '-' :
                                current_state = state::PRE_RELEASE;
                                return base_version;

                            case '+' :
                                current_state = state::BUILD_META_DATA;
                                return base_version;

                            default :
                                current_state = state::FINISHED;
                                return {};
                        }
                    }

                default :
                    return {};
            }
        }

        return {};
    };

    inline liba9n::option<const char *>
        semantic_version::parse_pre_release(const char *target_pre_release)
    {
        if (current_state != state::PRE_RELEASE)
        {
            return {};
        }

        if (auto result = string_to_value<'-'>(target_pre_release))
        {
            // copy char*
            // TODO: implementation this
        }

        return {};
    };

    inline liba9n::option<const char *>
        semantic_version::parse_build_meta_data(const char *target_build_meta_data)
    {
        if (current_state != state::BUILD_META_DATA)
        {
            return {};
        }

        if (auto result = string_to_value<'+'>(target_build_meta_data))
        {
            // copy char*
            // TODO: implementation this
        }

        return {};
    };

}

#endif
