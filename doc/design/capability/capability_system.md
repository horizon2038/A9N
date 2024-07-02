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

@endlml

## Capability Component
Capability Component is a Composite pattern. It enables unified operation of node and leaf.
Node has capability_component_slots with 2^radix capability_components, which contain pointers to entries or nodes.
Since capability_entry is usually used more, by default there is a 1:1 correspondence between the array of capability_entry and the slots.

## Unsolved
- How do we define a common interface for objects that need to track their children
(e.g. generic) ?
    - Define methods on local_state ?
    - Define a method on capability_component ?
        - add(free_descriptor, *capability_component)
