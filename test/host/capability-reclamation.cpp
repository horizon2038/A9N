#include <kernel/capability/capability_node.hpp>
#include <kernel/process/process.hpp>
#include <hal/interface/process_manager.hpp>
#include <stdio.h>
#include <stdlib.h>

using namespace a9n::kernel;

#define CHECK(expression) do { if (!(expression)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); abort(); \
} } while (0)

namespace
{
    class test_object : public capability_component
    {
      public:
        unsigned finalized = 0;
        bool fail = false;
        bool value_identity = false;
        capability_node *reclaiming_node = nullptr;
        bool is_same_object(const capability_slot &a, const capability_slot &b) const override
        {
            if (value_identity)
            {
                return a.data[0] == b.data[0];
            }
            return a.component == b.component;
        }
        capability_result execute(process &, capability_slot &) override { return {}; }
        capability_result revoke(capability_slot &) override
        {
            if (reclaiming_node)
            {
                CHECK(reclaiming_node->is_reclaiming());
                // Reclamation state must not corrupt lookup geometry, even
                // when a finalizer performs an internal lookup on that Node.
                auto slot = reclaiming_node->traverse_slot(a9n::word { 15 } << 60, 4, 0);
                CHECK(slot && slot.unwrap() == reclaiming_node->retrieve_slot(15).unwrap());
            }
            if (fail)
            {
                return capability_error::ILLEGAL_OPERATION;
            }
            ++finalized;
            return {};
        }
        capability_lookup_result retrieve_slot(a9n::word) override
        {
            return capability_lookup_error::TERMINAL;
        }
        capability_lookup_result traverse_slot(a9n::word, a9n::word, a9n::word) override
        {
            return capability_lookup_error::TERMINAL;
        }
    };

    void configure(capability_slot &slot, test_object &object, a9n::word identity = 0)
    {
        slot.init();
        slot.component = &object;
        slot.type = capability_type::IPC_PORT;
        slot.rights = capability_slot::ALL;
        slot.data[0] = identity;
    }

    struct directory
    {
        capability_slot slots[16] {};
        capability_node node { 0, 4, slots };
        capability_slot authority {};
        directory() { CHECK(try_configure_capability_node_slot(authority, node)); }
    };

    void peer_aliases()
    {
        CHECK(sizeof(capability_slot) == 64);
        test_object first, second;
        capability_slot root {}, a {}, b {}, c {}, other {}, moved {};
        configure(root, second);
        configure(a, first);
        root.insert_child(a);
        CHECK(try_copy_capability_slot(b, a));
        CHECK(try_copy_capability_slot(c, b));
        b.rights = capability_slot::WRITE;
        b.data[0] = 123;
        c.rights = capability_slot::NONE;
        CHECK(a.depth == b.depth && b.depth == c.depth);
        CHECK(a.is_same_object(b) && b.is_same_object(c));
        CHECK(a.has_sibling() && b.has_sibling() && c.has_sibling());
        CHECK(a.last_alias_in_group() == &c);
        CHECK(b.last_alias_in_group() == &c && c.last_alias_in_group() == &c);
        const capability_slot &constant_alias = a;
        CHECK(constant_alias.last_alias_in_group() == &c);
        configure(other, second);
        root.insert_child(other);
        CHECK(!other.has_sibling()); // same depth is not object identity
        CHECK(other.last_alias_in_group() == &other);
        CHECK(root.has_child());
        CHECK(a.try_remove_and_init());
        CHECK(first.finalized == 0 && b.has_sibling());
        CHECK(try_move_capability_slot(moved, b));
        CHECK(c.preview_slot == &moved && moved.next_slot == &c);
        CHECK(moved.try_remove_and_init());
        CHECK(first.finalized == 0 && !c.has_sibling());
        CHECK(c.try_remove_and_init());
        CHECK(first.finalized == 1);
        CHECK(other.try_remove_and_init());
        CHECK(!root.has_child());
    }

    void value_objects_and_failure()
    {
        test_object memory;
        memory.value_identity = true;
        capability_slot root {}, a {}, b {};
        configure(root, memory, 0x1000);
        configure(a, memory, 0x2000);
        configure(b, memory, 0x3000);
        root.insert_child(a);
        root.insert_child(b);
        CHECK(!a.is_same_object(b) && !a.has_sibling() && !b.has_sibling());
        memory.fail = true;
        CHECK(!root.try_revoke());
        CHECK(root.next_slot == &b && b.type != capability_type::NONE);
        memory.fail = false;
        CHECK(root.try_revoke());
        CHECK(!root.next_slot && a.type == capability_type::NONE && b.type == capability_type::NONE);
        CHECK(memory.finalized == 3);
    }

    void empty_slot_removal_preserves_its_neighbors()
    {
        test_object object;
        capability_slot parent {}, empty {}, next {};
        configure(parent, object);
        configure(next, object);
        parent.insert_child(next);
        parent.insert_child(empty);
        CHECK(parent.next_slot == &empty && empty.next_slot == &next);

        CHECK(capability_node::remove_slot(empty));
        CHECK(parent.next_slot == &next && next.preview_slot == &parent);
        CHECK(empty.type == capability_type::NONE && !empty.component);
        CHECK(!empty.next_slot && !empty.preview_slot && empty.depth == 0);
        CHECK(object.finalized == 0);

        // Reentrant anchors remain protected even if the Slot is empty.
        empty.reclaiming = true;
        CHECK(!capability_node::remove_slot(empty));
        CHECK(empty.reclaiming);
        empty.reclaiming = false;
        CHECK(capability_node::remove_slot(empty));
    }

    void independent_object_revoke_order()
    {
        struct ordered_object : test_object
        {
            unsigned &sequence;
            unsigned expected;
            bool fail = false;
            ordered_object(unsigned &counter, unsigned position)
                : sequence(counter), expected(position)
            {
            }
            capability_result revoke(capability_slot &self) override
            {
                CHECK(sequence == expected);
                CHECK(!self.has_child());
                if (fail)
                {
                    return capability_error::FATAL;
                }
                ++sequence;
                ++finalized;
                return {};
            }
        };
        unsigned sequence = 0;
        ordered_object parent(sequence, 2), child(sequence, 1), grandchild(sequence, 0);
        capability_slot root {}, middle {}, leaf {};
        configure(root, parent);
        root.type = capability_type::GENERIC;
        configure(middle, child);
        configure(leaf, grandchild);
        root.insert_child(middle);
        middle.insert_child(leaf);
        grandchild.fail = true;
        CHECK(!capability_node::revoke_slot(root));
        CHECK(sequence == 0 && parent.finalized == 0 && child.finalized == 0);
        CHECK(root.has_child() && middle.has_child());
        CHECK(!root.reclaiming && !middle.reclaiming && !leaf.reclaiming);
        grandchild.fail = false;
        CHECK(capability_node::revoke_slot(root));
        CHECK(sequence == 3 && parent.finalized == 1);
        CHECK(root.type == capability_type::GENERIC && !root.has_child());
        CHECK(middle.type == capability_type::NONE && leaf.type == capability_type::NONE);
    }

    void contained_foreign_aliases()
    {
        directory d;
        test_object external;
        capability_slot original {};
        configure(original, external);
        CHECK(try_copy_capability_slot(d.slots[0], original));
        CHECK(try_copy_capability_slot(d.slots[1], original));
        d.slots[1].rights = capability_slot::NONE;
        CHECK(d.authority.try_revoke());
        CHECK(d.slots[0].type == capability_type::NONE && d.slots[1].type == capability_type::NONE);
        CHECK(external.finalized == 0 && original.component == &external);
        CHECK(original.try_remove_and_init());
        CHECK(external.finalized == 1);
    }

    void alias_group_children_and_active_anchor()
    {
        test_object parent_object, child_object;
        capability_slot parent {}, alias {}, child {};
        configure(parent, parent_object);
        configure(child, child_object);
        CHECK(try_copy_capability_slot(alias, parent));
        parent.insert_child(child);
        CHECK(parent.next_slot == &alias && alias.next_slot == &child);
        CHECK(parent.has_child() && alias.has_child());
        CHECK(parent.try_remove_and_init());
        CHECK(alias.try_revoke());
        CHECK(child_object.finalized == 1 && child.type == capability_type::NONE);

        // A Generic stored inside a Node it generated cannot destroy its own
        // active traversal anchor. Failure must retain its allocation state;
        // moving that authority outside the contained Node makes retry safe.
        directory d;
        test_object allocation;
        configure(d.slots[0], allocation);
        d.slots[0].type = capability_type::GENERIC;
        d.slots[0].insert_child(d.authority);
        CHECK(!d.slots[0].try_revoke());
        CHECK(allocation.finalized == 0 && d.slots[0].component == &allocation);
        capability_slot outside {};
        CHECK(try_move_capability_slot(outside, d.slots[0]));
        CHECK(outside.try_revoke());
        CHECK(allocation.finalized == 1 && d.authority.type == capability_type::NONE);
    }

    void cycles()
    {
        directory a, b;
        CHECK(try_copy_capability_slot(a.slots[0], a.authority));
        CHECK(try_copy_capability_slot(a.slots[1], b.authority));
        CHECK(try_copy_capability_slot(b.slots[0], a.authority));
        CHECK(try_copy_capability_slot(b.slots[1], b.authority));
        CHECK(a.authority.try_revoke());
        CHECK(a.slots[0].type == capability_type::NONE && a.slots[1].type == capability_type::NONE);
        CHECK(b.authority.type == capability_type::NODE);
        CHECK(b.authority.try_revoke());
        CHECK(b.slots[0].type == capability_type::NONE && b.slots[1].type == capability_type::NONE);
        // No external aliases remain: destroy the mutually containing Nodes.
        CHECK(try_copy_capability_slot(a.slots[0], b.authority));
        CHECK(try_copy_capability_slot(b.slots[0], a.authority));
        CHECK(a.authority.try_remove_and_init());
        CHECK(b.authority.try_remove_and_init());
        CHECK(a.slots[0].try_remove_and_init());
        CHECK(a.slots[0].type == capability_type::NONE && b.slots[0].type == capability_type::NONE);
    }

    void node_authority()
    {
        directory d;
        test_object object;
        process owner {};
        ipc_buffer buffer {};
        owner.buffer = &buffer;
        configure(d.slots[0], object);
        d.slots[0].rights = capability_slot::WRITE;
        buffer.messages[1] = 4; // DEMOTE
        buffer.messages[2] = 0;
        buffer.messages[4] = 0;
        CHECK(d.node.execute(owner, d.authority));
        CHECK(d.slots[0].rights == 0);
        buffer.messages[1] = 5; // A weak alias must not reset a shared object.
        d.authority.rights = capability_slot::READ;
        CHECK(!d.node.execute(owner, d.authority));
        d.authority.rights = capability_slot::ALL;
        CHECK(!d.node.execute(owner, d.authority));
        buffer.messages[1] = 6; // REMOVE
        d.authority.rights = capability_slot::READ;
        CHECK(!d.node.execute(owner, d.authority));
        d.authority.rights = capability_slot::ALL;
        CHECK(d.node.execute(owner, d.authority));
        CHECK(d.slots[0].type == capability_type::NONE);
        CHECK(d.node.retrieve_slot(0).unwrap() == &d.slots[0]);
        CHECK(d.node.retrieve_slot(15).unwrap() == &d.slots[15]);
    }

    void node_transfer_operations()
    {
        directory d;
        test_object object;
        process owner {};
        ipc_buffer buffer {};
        owner.buffer = &buffer;
        CHECK(try_configure_capability_node_slot(owner.root_slot, d.node));
        configure(d.slots[0], object);
        buffer.messages[3] = a9n::word { 4 } << 56; // Root Node's slot zero, depth four.

        buffer.messages[1] = 1; // COPY
        buffer.messages[2] = 1;
        CHECK(d.node.execute(owner, d.authority));
        CHECK(d.slots[0].is_same_object(d.slots[1]));
        CHECK(d.slots[1].rights == capability_slot::ALL && d.slots[1].depth == d.slots[0].depth);

        buffer.messages[1] = 3; // MINT
        buffer.messages[2] = 2;
        buffer.messages[4] = capability_slot::WRITE;
        CHECK(d.node.execute(owner, d.authority));
        CHECK(d.slots[2].rights == capability_slot::WRITE && d.slots[2].is_same_object(d.slots[0]));

        buffer.messages[1] = 2; // MOVE needs no COPY right.
        buffer.messages[2] = 3;
        buffer.messages[3] |= a9n::word { 2 } << 52;
        CHECK(d.node.execute(owner, d.authority));
        CHECK(d.slots[2].type == capability_type::NONE && d.slots[3].rights == capability_slot::WRITE);

        CHECK(d.slots[3].component == &object);

        buffer.messages[2] = 4;
        buffer.messages[3] = (a9n::word { 4 } << 56) | (a9n::word { 3 } << 52);
        const a9n::word copy_operations[] { 1, 3 };
        for (a9n::word operation : copy_operations)
        {
            buffer.messages[1] = operation;
            auto failed = d.node.execute(owner, d.authority);
            CHECK(!failed && failed.unwrap_error() == capability_error::PERMISSION_DENIED);
            CHECK(d.slots[4].type == capability_type::NONE);
        }
        buffer.messages[1] = 3;
        buffer.messages[3] = a9n::word { 4 } << 56;
        buffer.messages[4] = a9n::word { 1 } << 8; // Cannot grant a right absent from the source.
        CHECK(!d.node.execute(owner, d.authority));
        buffer.messages[4] = capability_slot::READ;
        d.authority.rights = capability_slot::READ;
        CHECK(!d.node.execute(owner, d.authority));
    }

    void node_geometry_is_checked_at_creation()
    {
        CHECK(sizeof(capability_node) == 32);
        static_assert(capability_node::is_geometry_valid(0, 4));
        static_assert(capability_node::is_geometry_valid(56, 8));
        static_assert(!capability_node::is_geometry_valid(0, 0));
        static_assert(!capability_node::is_geometry_valid(0, 64));
        static_assert(!capability_node::is_geometry_valid(64, 1));
        static_assert(!capability_node::is_geometry_valid(63, 2));
        capability_slot slots[16] {};
        capability_slot authority {};
        const a9n::word invalid[][2] { { 0, 0 }, { 0, 64 }, { 64, 4 }, { 63, 4 },
                                     { 0, (a9n::word { 1 } << 32) + 4 } };
        for (const auto &geometry : invalid)
        {
            capability_node node(geometry[0], geometry[1], slots);
            CHECK(!node.is_valid());
            CHECK(!try_configure_capability_node_slot(authority, node));
            CHECK(authority.type == capability_type::NONE);
            CHECK(!node.retrieve_slot(0));
            CHECK(slots[0].type == capability_type::NONE);
        }
        capability_node missing(0, 4, nullptr);
        CHECK(!missing.is_valid());
        auto unavailable = missing.traverse_slot(0, 4, 0);
        CHECK(!unavailable && unavailable.unwrap_error() == capability_lookup_error::UNAVAILABLE);
        capability_node node(0, 4, slots);
        CHECK(try_configure_capability_node_slot(authority, node));
        for (a9n::word index = 0; index < 16; ++index)
        {
            auto descriptor = (index << 60) | ((a9n::word { 1 } << 60) - 1);
            auto found = node.traverse_slot(descriptor, 4, 0);
            CHECK(found && found.unwrap() == &slots[index]);
        }
    }

    void monadic_node_traversal()
    {
        capability_slot root_slots[16] {}, branch_slots[16] {}, leaf_slots[16] {};
        capability_node root(8, 4, root_slots);
        capability_node branch(4, 4, branch_slots);
        capability_node leaf(32, 4, leaf_slots);
        CHECK(try_configure_capability_node_slot(root_slots[15], branch));
        CHECK(try_configure_capability_node_slot(branch_slots[15], leaf));

        auto descriptor = ~a9n::word { 0 };
        CHECK(root.traverse_slot(descriptor, 20, 8).unwrap() == &root_slots[15]);
        CHECK(root.traverse_slot(descriptor, 28, 8).unwrap() == &branch_slots[15]);
        for (a9n::word index = 0; index < 16; ++index)
        {
            auto found = root.traverse_slot((descriptor & ~a9n::word { 15 }) | index, 64, 8);
            CHECK(found && found.unwrap() == &leaf_slots[index]);
        }

        root_slots[15].init();
        auto empty = root.traverse_slot(descriptor, 64, 8);
        CHECK(!empty && empty.unwrap_error() == capability_lookup_error::EMPTY);
        CHECK(root.traverse_slot(descriptor, 20, 8).unwrap() == &root_slots[15]);
    }

    void reclamation_state_is_not_lookup_geometry()
    {
        directory d;
        test_object object;
        configure(d.slots[0], object);
        object.reclaiming_node = &d.node;
        object.fail = true;
        CHECK(!d.authority.try_revoke());
        CHECK(!d.node.is_reclaiming());
        CHECK(d.slots[0].component == &object);
        object.fail = false;
        CHECK(d.authority.try_revoke());
        CHECK(!d.node.is_reclaiming());
        CHECK(d.slots[0].type == capability_type::NONE);
    }
}

namespace a9n::hal
{
    liba9n::result<a9n::word, hal_error> get_message_register(const process &target, a9n::word index)
    {
        CHECK(index < 64 && target.buffer);
        return target.buffer->messages[index];
    }
}

int main()
{
    peer_aliases();
    value_objects_and_failure();
    empty_slot_removal_preserves_its_neighbors();
    independent_object_revoke_order();
    contained_foreign_aliases();
    alias_group_children_and_active_anchor();
    cycles();
    node_authority();
    node_transfer_operations();
    node_geometry_is_checked_at_creation();
    monadic_node_traversal();
    reclamation_state_is_not_lookup_geometry();
    puts("Capability reclamation: peers, value identities, weak rights, Nodes, cycles, failure and lookup PASS");
}
