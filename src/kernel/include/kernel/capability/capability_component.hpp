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

    struct alignas(a9n::WORD_BITS) capability_slot
    {
        enum object_rights : uint8_t
        {
            NONE   = 0,
            READ   = 1 << 0,
            WRITE  = 1 << 1,
            COPY   = 1 << 2,
            MODIFY = 1 << 3,
            // MOVE is always allowed
            ALL = READ | WRITE | COPY | MODIFY,
        };

        capability_component *component;

        // padding to a9n::word width
        union
        {
            a9n::word reserved;

            struct
            {
                capability_type type { capability_type::NONE };
                uint8_t         rights { object_rights::NONE };
            };
        };

        capability_slot_data data;
        capability_slot     *next_slot;
        capability_slot     *preview_slot;

        // child capability_entry
        // capability_entry *child_capability_entry;
        a9n::word depth;

        void init(void)
        {
            component = nullptr;
            type      = capability_type::NONE;
            rights    = object_rights::NONE;
            data.fill(0);
            next_slot    = nullptr;
            preview_slot = nullptr;
            depth        = 0;
        }

        kernel_result try_remove_and_init(void);

        void remove_from_dependency_node(void)
        {
            if (preview_slot)
            {
                preview_slot->next_slot = next_slot;
            }

            if (next_slot)
            {
                next_slot->preview_slot = preview_slot;
            }
        }

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
            if (!next_slot || !preview_slot)
            {
                return false;
            }

            return (depth == next_slot->depth) || (depth == preview_slot->depth);
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
            next_slot         = &slot;

            slot.preview_slot = this;
            slot.next_slot    = temp_next_slot;
            // slot.depth        = depth + 1;

            // configure old next slot
            if (!temp_next_slot)
            {
                return;
            }
            temp_next_slot->preview_slot = &slot;
        }
    };

    enum class capability_lookup_error : a9n::word
    {
        TERMINAL,
        INDEX_OUT_OF_RANGE,
        UNAVAILABLE,
        EMPTY,
        UNEXPECTED,
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
        virtual capability_result revoke(capability_slot &self) = 0;

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

    inline kernel_result capability_slot::try_remove_and_init(void)
    {
        if (type == capability_type::NONE)
        {
            // already initialized
            return {};
        }

        if (!(rights & object_rights::READ) || !(rights & object_rights::WRITE))
        {
            return kernel_error::PERMISSION_DENIED;
        }

        if (has_sibling())
        {
            remove_from_dependency_node();
            init();

            return {};
        }

        if (component)
        {
            auto result = component->revoke(*this);
            if (!result)
            {
                return kernel_error::UNEXPECTED;
            }
        }

        remove_from_dependency_node();
        init();

        return {};
    }

    inline kernel_result try_move_capability_slot(capability_slot &destination, capability_slot &source)
    {
        if (destination.type != capability_type::NONE)
        {
            return kernel_error::INIT_FIRST;
        }

        destination.init();

        destination.type      = source.type;
        destination.data      = source.data;
        destination.component = source.component;
        destination.rights    = source.rights;

        // dependency node re-configuration
        destination.depth        = source.depth;
        destination.next_slot    = source.next_slot;
        destination.preview_slot = source.preview_slot;

        if (destination.preview_slot)
        {
            destination.preview_slot->next_slot = &destination;
        }

        if (destination.next_slot)
        {
            destination.next_slot->preview_slot = &destination;
        }

        source.init();

        return {};
    }

    inline kernel_result try_copy_capability_slot(capability_slot &destination, capability_slot &source)
    {
        if (destination.type != capability_type::NONE)
        {
            return kernel_error::INIT_FIRST;
        }

        destination.init();

        destination.type      = source.type;
        destination.data      = source.data;
        destination.component = source.component;
        destination.rights    = source.rights;

        source.insert_sibling(destination);

        return {};
    }
}

#endif
