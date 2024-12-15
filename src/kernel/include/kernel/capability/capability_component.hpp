#ifndef CAPABILITY_COMPONENT_HPP
#define CAPABILITY_COMPONENT_HPP

#include <kernel/capability/capability_local_state.hpp>
#include <kernel/capability/capability_result.hpp>
#include <kernel/capability/capability_types.hpp>
#include <kernel/ipc/ipc_buffer.hpp>
#include <kernel/types.hpp>

#include <liba9n/libc/string.hpp>

namespace a9n::kernel
{

    class capability_component;

    struct capability_slot_state
    {
    };

    struct alignas(a9n::WORD_BITS) capability_slot
    {
        capability_component *component;

        // padding to a9n::word width
        union
        {
            a9n::word       reserved;
            capability_type type { capability_type::NONE };
        };

        capability_slot_data data;
        capability_slot     *next_slot;
        capability_slot     *preview_slot;

        // child capability_entry
        // capability_entry *child_capability_entry;
        a9n::word depth;

        bool has_child(void) const
        {
            if (!next_slot)
            {
                return false;
            }

            return (depth > next_slot->depth);
        }

        bool has_sibling(void) const
        {
            if (!next_slot)
            {
                return false;
            }

            return (depth == next_slot->depth);
        }

        bool is_same_slot(const capability_slot &rhs) const;

        void insert_child(capability_slot &child_slot)
        {
            child_slot.depth = depth + 1;
            insert(child_slot);
        }

        void insert_sibling(capability_slot &sibling_slot)
        {
            sibling_slot.depth = depth;
            insert(sibling_slot);
        }

        void insert(capability_slot &slot)
        {
            // +--------+    +--------+    +--------+
            // |        | -> |        | -> |        |
            // |  this  |    |  slot  |    |  next  |
            // |        | <- |        | <- |        |
            // +--------+    +--------+    +--------+

            auto temp_next_slot = next_slot;

            // configure this
            next_slot = &slot;

            // TODO: consider the case where the dependency node of the target_slot
            // has already been set
            slot.preview_slot = this;
            slot.next_slot    = temp_next_slot;
            slot.depth        = depth + 1;

            // configure old next slot
            if (!temp_next_slot)
            {
                return;
            }
            temp_next_slot->preview_slot = &slot;
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

    using capability_lookup_result = liba9n::result<capability_slot *, capability_lookup_error>;

    inline constexpr const char *capability_lookup_error_to_string(capability_lookup_error error)
    {
        switch (error)
        {
            case capability_lookup_error::TERMINAL :
                return "TERMINAL";

            case capability_lookup_error::INDEX_OUT_OF_RANGE :
                return "INDEX OUT OF RANGE";

            case capability_lookup_error::UNAVAILABLE :
                return "UNAVAILABLE";

            case capability_lookup_error::EMPTY :
                return "EMPTY";

            default :
                return "ERROR NOT FOUND";
        }
    };

    // src/kernel/process/process.hpp
    class process;

    class capability_component
    {
      public:
        // called from user
        virtual capability_result execute(process &this_process, capability_slot &this_slot) = 0;

        // called from node
        virtual capability_result revoke() = 0;

        virtual bool
            is_same_slot(const capability_slot &this_slot, const capability_slot &target_slot) const
        {
            return false;
        }

        // for tree
        virtual capability_lookup_result retrieve_slot(a9n::word index) = 0;

        virtual capability_lookup_result traverse_slot(
            a9n::capability_descriptor descriptor,
            a9n::word                  descriptor_max_bits,
            a9n::word                  descriptor_used_bits
        ) = 0;
    };

    inline bool capability_slot::is_same_slot(const capability_slot &rhs) const
    {
        if (!rhs.component)
        {
            return false;
        }

        return rhs.component->is_same_slot(rhs, *this);
    }
}

#endif
