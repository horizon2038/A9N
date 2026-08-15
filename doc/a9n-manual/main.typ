/* a9n manual entry point */

#import "/components/template.typ" : template

#show: template.with(
    title: "A9N Manual",
    version: "0.1.15",
    author: "Rekka 'horizon' IGUMI",
    date: datetime(year: 2026, month: 8, day: 15),
)

#include "/pages/introduction.typ"


/*
#template(
    title: "A9N Manual",
    version: "0.1.15",
    author: "Rekka 'horizon' IGUMI",
    [
        #include "/pages/introduction.typ"
        #include "/pages/kernel_call.typ"
        #include "/pages/capability.typ"
        #include "/pages/node.typ"
        #include "/pages/generic.typ"
        #include "/pages/process_control_block.typ"
        #include "/pages/ipc_port.typ"
        #include "/pages/notification_port.typ"
        #include "/pages/interrupt_region.typ"
        #include "/pages/interrupt_port.typ"
        #include "/pages/io_port.typ"
        #include "/pages/address_space.typ"
        #include "/pages/page_table.typ"
        #include "/pages/frame.typ"
        #include "/pages/virtual_cpu.typ"
        #include "/pages/virtual_address_space.typ"
        #include "/pages/virtual_page_table.typ"
        #include "/pages/protocol.typ"
        #include "/pages/abi.typ"
        #include "/pages/porting.typ"
    ],
)
*/
