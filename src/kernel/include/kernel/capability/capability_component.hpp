#ifndef CAPABILITY_COMPONENT_HPP
#define CAPABILITY_COMPONENT_HPP

#include <kernel/capability/capability_local_state.hpp>
#include <kernel/capability/capability_result.hpp>
#include <kernel/ipc/ipc_buffer.hpp>
#include <kernel/types.hpp>

#include <liba9n/libc/string.hpp>

namespace a9n::kernel
{
    class capability_component;

    struct alignas(a9n::WORD_BITS) capability_slot
    {
        capability_component *component;
        a9n::word             authority;

        capability_slot_data data;
        dependency_node      family_node;

        bool has_child()
        {
            return (family_node.depth > family_node.next_slot->family_node.depth);
        }

        bool is_same_slot(capability_slot *rhs)
        {
            auto is_same_data = liba9n::std::memcmp(
                &data,
                &rhs->data,
                sizeof(capability_slot_data)
            );

            if (is_same_data != 0)
            {
                return false;
            }

            // for memory object (e.g. generic, frame, page_table ...)
            if (component != rhs->component)
            {
                return false;
            }

            return true;
        }

        template<a9n::word Index>
            requires(Index < ENTRY_DATA_MAX)
        constexpr a9n::word get_local_data()
        {
            return data[Index];
        }

        template<a9n::word Index>
            requires(Index < ENTRY_DATA_MAX)
        constexpr a9n::word get_local_data() const
        {
            return data[Index];
        }

        constexpr a9n::word get_local_data(a9n::word index)
        {
            return data[index];
        }

        constexpr a9n::word get_local_data(a9n::word index) const
        {
            return data[index];
        }

        template<a9n::word Index>
            requires(Index < ENTRY_DATA_MAX)
        constexpr void set_local_data(a9n::word value)
        {
            data[Index] = value;
        }

        constexpr void set_local_data(a9n::word index, a9n::word value)
        {
            data[index] = value;
        }
    };

    enum class capability_lookup_error : a9n::word
    {
        TERMINAL,
        INDEX_OUT_OF_RANGE,
        UNAVAILABLE,
        EMPTY,
    };

    using capability_lookup_result
        = liba9n::result<capability_slot *, capability_lookup_error>;

    class capability_component
    {
      public:
        // called from user
        virtual capability_result execute(
            ipc_buffer      *buffer,
            capability_slot *this_slot,
            capability_slot *root_slot
        ) = 0;

        // called from node
        virtual capability_result revoke() = 0;

        // for tree
        virtual capability_lookup_result retrieve_slot(a9n::word index) = 0;

        virtual capability_lookup_result traverse_slot(
            a9n::capability_descriptor descriptor,
            a9n::word                  descriptor_max_bits,
            a9n::word                  descriptor_used_bits
        ) = 0;
    };
}

#endif
