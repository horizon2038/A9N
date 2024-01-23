# Capability System

@startuml

title capability_architecture

rectangle composite {

    interface capability <<composite>>
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

capability_node .u.|> capability
capability_entry .u.|> capability

capability_node o-r-> capability : 0..n
capability_object <-r- capability_entry

generic .u.|> capability_object
process_control_block .u.|> capability_object
ipc_port .u.|> capability_object
frame .u.|> capability_object

generic -u-> capability

@enduml
