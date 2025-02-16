/* a9n manual entry point */

#import "/components/layout.typ" : template

#template(
    title: "A9N Manual",
    author: "horizon2k38",
    version: "1.0.0",
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
