#ifndef LIBA9N_VERSION_HPP
#define LIBA9N_VERSION_HPP

#include <liba9n/option/option.hpp>

#include <stdint.h>

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

        static constexpr uintmax_t IDENTIFIER_BUFFER_SIZE = 32;

        uintmax_t major { 0 };
        uintmax_t minor { 0 };
        uintmax_t patch { 0 };

        char pre_release[IDENTIFIER_BUFFER_SIZE] { 0 };
        char build_meta_data[IDENTIFIER_BUFFER_SIZE] { 0 };

        static bool is_identifier_character(char character)
        {
            return (character >= '0' && character <= '9') || (character >= 'A' && character <= 'Z')
                || (character >= 'a' && character <= 'z') || character == '-' || character == '.';
        }

        static liba9n::option<const char *> copy_until_separator(
            char       *destination,
            uintmax_t   destination_size,
            const char *source,
            char        separator
        )
        {
            if (!destination || !source || destination_size == 0)
            {
                return {};
            }

            uintmax_t index = 0;

            while (*source != '\0' && *source != separator)
            {
                if (!is_identifier_character(*source))
                {
                    return {};
                }

                if (index + 1 >= destination_size)
                {
                    return {};
                }

                destination[index] = *source;
                index++;
                source++;
            }

            if (index == 0)
            {
                return {};
            }

            destination[index] = '\0';

            if (*source == separator)
            {
                source++;
            }

            return source;
        }

        liba9n::option<const char *> parse_base(const char *base_version);
        liba9n::option<const char *> parse_pre_release(const char *target_pre_release);
        liba9n::option<const char *> parse_build_meta_data(const char *target_build_meta_data);
    };

    inline semantic_version::semantic_version(const char *version)
    {
        auto captured_parse_pre_release =
            [this](const char *target_pre_release) -> liba9n::option<const char *>
        {
            return parse_pre_release(target_pre_release);
        };

        auto captured_parse_build_meta_data =
            [this](const char *target_build_meta_data) -> liba9n::option<const char *>
        {
            return parse_build_meta_data(target_build_meta_data);
        };

        parse_base(version).and_then(captured_parse_pre_release).and_then(captured_parse_build_meta_data);
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
        minor = target_minor;
        patch = target_patch;

        if (target_pre_release && *target_pre_release)
        {
            copy_until_separator(pre_release, IDENTIFIER_BUFFER_SIZE, target_pre_release, '\0');
        }

        if (target_build_meta_data && *target_build_meta_data)
        {
            copy_until_separator(build_meta_data, IDENTIFIER_BUFFER_SIZE, target_build_meta_data, '\0');
        }

        current_state = state::FINISHED;
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
                    {
                        if (auto result = string_to_value<'.'>(base_version))
                        {
                            major         = result.unwrap().value;
                            base_version  = result.unwrap().current;
                            current_state = state::MINOR;
                            break;
                        }

                        return {};
                    }

                case state::MINOR :
                    {
                        if (auto result = string_to_value<'.'>(base_version))
                        {
                            minor         = result.unwrap().value;
                            base_version  = result.unwrap().current;
                            current_state = state::PATCH;
                            break;
                        }

                        return {};
                    }

                case state::PATCH :
                    {
                        if (auto result = string_to_value<'-', '+'>(base_version))
                        {
                            patch            = result.unwrap().value;

                            const char *next = result.unwrap().current;

                            if (next > base_version && *(next - 1) == '-')
                            {
                                current_state = state::PRE_RELEASE;
                                return next;
                            }

                            if (next > base_version && *(next - 1) == '+')
                            {
                                current_state = state::BUILD_META_DATA;
                                return next;
                            }

                            if (*next == '\0')
                            {
                                current_state = state::FINISHED;
                                return {};
                            }
                        }

                        return {};
                    }

                default :
                    return {};
            }
        }

        return {};
    }

    inline liba9n::option<const char *>
        semantic_version::parse_pre_release(const char *target_pre_release)
    {
        if (current_state != state::PRE_RELEASE)
        {
            return {};
        }

        if (auto result
            = copy_until_separator(pre_release, IDENTIFIER_BUFFER_SIZE, target_pre_release, '+'))
        {
            const char *current = result.unwrap();

            if (current > target_pre_release && *(current - 1) == '+')
            {
                current_state = state::BUILD_META_DATA;
                return current;
            }

            current_state = state::FINISHED;
            return {};
        }

        return {};
    }

    inline liba9n::option<const char *>
        semantic_version::parse_build_meta_data(const char *target_build_meta_data)
    {
        if (current_state != state::BUILD_META_DATA)
        {
            return {};
        }

        if (auto result
            = copy_until_separator(build_meta_data, IDENTIFIER_BUFFER_SIZE, target_build_meta_data, '\0'))
        {
            current_state = state::FINISHED;
            return result.unwrap();
        }

        return {};
    }
}

#endif
