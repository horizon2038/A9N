#import "@preview/bytefield:0.0.6": *
#import "/components/api_table.typ" : *

= Process Control Block

== Introduction

Process Control Blockは実行可能なContextを表すCapabilityです. 他KernelにおけるThreadに相当します.

Hardware Context (Register Set)とTime Quantum, Priority, 付随するCapabilityなどが1つにパッケージされています.

== Process Control Block API

/*
enum operation_type : a9n::word
{
    NONE,
    CONFIGURE,
    READ_REGISTER,
    WRITE_REGISTER,
    RESUME,
    SUSPEND,
};

enum operation_configure_index : a9n::word
{
    CONFIGURE_RESERVED = OPERATION_TYPE,

    CONFIGURATION_INFO,

    // space
    ROOT_PAGE_TABLE_DESCRIPTOR,
    ROOT_NODE_DESCRIPTOR,

    // ipc
    FRAME_IPC_BUFFER_DESCRIPTOR,
    NOTIFICATION_PORT,

    // resolver (fault handler)
    IPC_PORT_RESOLVER,

    // general purpose registers
    INSTRUCTION_POINTER,
    STACK_POINTER,
    THREAD_LOCAL_BASE,

    // scheduling
    PRIORITY,
    AFFINITY, // SMP only
};
*/

=== `configure`

Process Control Blockの設定を行います.
不必要なCopyを避けるため, Configuration Infoによって設定項目を指定します.

==== `configuration_info`

#bytefield(
    bpr: 16,
    rows: (14em),
    bitheader(
        "bounds",
        0,
        8,
        15,
        text-size: 8pt,
    ),

    flag[ROOT_PAGE_TABLE],
    flag[ROOT_NODE],
    flag[FRAME_IPC_BUFFER],
    flag[NOTIFICATION_PORT],
    flag[IPC_PORT_RESOLVER],
    flag[INSTRUCTION_POINTER],
    flag[STACK_POINTER],
    flag[THREAD_LOCAL_BASE],
    flag[PRIORITY],
    flag[AFFINITY],
    bits(6)[RESERVED],

    text-size: 4pt,
)

#api_table(
    "capability_descriptor", "process_control_block", "対象Process Control BlockへのDescriptor",
    "configuration_info", "info", "設定する項目",
    "capability_descriptor", "root_page_table", "Root Page TableへのDescriptor",
    "capability_descriptor", "root_node", "Root NodeへのDescriptor",
    "capability_descriptor", "frame_ipc_buffer", "IPC BufferとしたいFrameへのDescriptor",
    "capability_descriptor", "notification_port", "Notification PortへのDescriptor",
    "capability_descriptor", "ipc_port_resolver", "ResolverとしたいIPC PortのDescriptor",
    "virtual_address", "instruction_pointer", "Instruction Pointer",
    "virtual_address", "stack_pointer", "Stack Pointer",
    "virtual_address", "thread_local_base", "Thread Local Base",
    "word", "priority", "優先度",
    "word", "affinity", "SMP環境におけるAffinity (CPU CoreのIndex)",
)

=== `read_register`

Registerを読み出します.

#api_table(
    "descriptor", "process_control_block", "対象Process Control BlockへのDescriptor",
    "word", "register_count", "読み出すRegisterの数",
)

=== `write_register`

Registerを書き込みます.

#api_table(
    "descriptor", "process_control_block", "対象Process Control BlockへのDescriptor",
    "word", "register_count", "書き込むRegisterの数",
)

=== `resume`

Process Control BlockをScheduler QueueにPushし, 実行可能状態にします.

#api_table(
    "descriptor", "process_control_block", "対象Process Control BlockへのDescriptor",
)

=== `suspend`

Process Control Blockを実行不可能状態にします.

#api_table(
    "descriptor", "process_control_block", "対象Process Control BlockへのDescriptor",
)


