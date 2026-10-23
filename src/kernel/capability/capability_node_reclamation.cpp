#include <kernel/capability/capability_node.hpp>

namespace a9n::kernel
{
    // Keep the existing Slot entry points; the Node layer owns reclamation.
    kernel_result capability_slot::try_revoke(void)
    {
        return capability_node::revoke_slot(*this);
    }

    kernel_result capability_slot::try_remove_and_init(void)
    {
        return capability_node::remove_slot(*this);
    }

    kernel_result capability_node::revoke_slot(capability_slot &slot)
    {
        if (slot.type == capability_type::NONE || !slot.component || slot.reclaiming)
        {
            return kernel_error::ILLEGAL_ARGUMENT;
        }

        struct reclamation_guard
        {
            capability_slot &slot;

            explicit reclamation_guard(capability_slot &value) : slot(value)
            {
                slot.reclaiming = true;
            }

            ~reclamation_guard()
            {
                slot.reclaiming = false;
            }
        } guard(slot);

        // COPY/MINT aliases are peers. Generated children follow the alias
        // group; their objects are reclaimed independently, before the parent.
        auto *last_alias = slot.last_alias_in_group();
        while (last_alias->next_slot && last_alias->next_slot->depth > slot.depth)
        {
            // A Node finalizer may also unlink contained slots. Never retain
            // a descendant pointer across another object's revoke call.
            auto remove_result = remove_slot(*last_alias->next_slot);
            if (!remove_result)
            {
                return remove_result;
            }
        }

        if (!slot.component->revoke(slot))
        {
            return kernel_error::UNEXPECTED;
        }
        return {};
    }

    kernel_result capability_node::remove_slot(capability_slot &slot)
    {
        auto unlink_and_clear_slot = [&slot]() -> kernel_result
        {
            slot.remove_from_dependency_node();
            slot.init();
            return {};
        };

        // Reentrant removal is only allowed to break an active Node cycle.
        if (slot.reclaiming)
        {
            if (!slot.component || !slot.component->is_reclaiming())
            {
                // Other active traversal anchors must remain pinned.
                return kernel_error::ILLEGAL_ARGUMENT;
            }
            return unlink_and_clear_slot();
        }

        // An empty Slot has no Object to reclaim, but may still be linked.
        if (slot.type == capability_type::NONE)
        {
            return unlink_and_clear_slot();
        }

        // Another alias keeps the shared Object alive; remove only this Slot.
        if (slot.has_sibling())
        {
            return unlink_and_clear_slot();
        }

        // The final alias owns Object teardown. Retain it if cleanup fails.
        if (slot.component)
        {
            auto revoke_result = revoke_slot(slot);
            if (!revoke_result)
            {
                return kernel_error::UNEXPECTED;
            }
        }

        return unlink_and_clear_slot();
    }
}
