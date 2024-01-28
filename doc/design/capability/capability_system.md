# Capability System

@startuml

title capability_architecture

rectangle composite {

    interface capability_component <<component>>
    {
        + execute(operation)
        + revoke()
        + traverse(descriptor)
    }

    class capability_entry <<leaf>>
    {
        - capability_object_pointer
        - entry_local_state
        + execute(operation)
        + revoke()
    }

    class capability_node <<node>>
    {
        + execute(operation)
        + traverse(descriptor)
    }

}

struct entry_local_state
{
    + entry_data
    + dependency_node
}

capability_entry -> entry_local_state

interface capability_object
{
    + execute(operation, local_state)
    + revoke()
}

class generic
{
}

class process_control_block
{
}

class ipc_port
{
}

class frame
{
}

capability_node .u.|> capability_component
capability_entry .u.|> capability_component

capability_node o-r-> capability_component : 0..n
capability_object <-r- capability_entry

generic .u.|> capability_object
process_control_block .u.|> capability_object
ipc_port .u.|> capability_object
frame .u.|> capability_object

generic -u-> capability_component

@enduml

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
