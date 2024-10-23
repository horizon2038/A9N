# Capability System

@startuml

title capability_architecture

skinparam linetype ortho

rectangle composite {
    interface capability_component <<component>>
    {
        + execute()
        + revoke()

        + traverse_slot()
        + retrieve_slot()
    }

    struct capability_slot<<leaf>>
    {
        + local_state
    }

    class capability_node<<node>>
    {
    }
}

rectangle memory {
    class generic
    {
    }

    class frame
    {
    }

    class page_table
    {
    }
}


class process_control_block
{
}

class ipc_port
{
}

capability_node o-r-> capability_slot : 0..n

capability_node .u.|> capability_component

capability_slot -u> capability_component

frame -[hidden]-> ipc_port
page_table -[hidden]-> process_control_block

generic .l.|> capability_component
process_control_block .l.|> capability_component
ipc_port .l.|> capability_component
frame .l.|> capability_component
page_table .l.|> capability_component

@enduml

Capability is composed of a Composite Pattern.
This structure makes it possible to search Capability in a *Type-Safe* and *Object-Oriented* manner.

> [!NOTE]
> Down Cast is needed when a Capability wantas to Access another Capability from its internal implementation.

## Capability Slot

Every Capability is an implementation of Capability Component.
Its pointer is stored in *Capability Slot*.
Slot contains *Slot-Local Data*, and a single *Capability-Component* is used to realize Capability of memory system that does not need to have an entity.

## Capability Node

*Capability Node* is like a *directory* with `2^radix_bits` of *Capability Slots*.
Nodes can be nested because Capability *Node* is also a Capability.
