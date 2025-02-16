#include <kernel/capability/interrupt_region.hpp>

#include <hal/interface/process_manager.hpp>

namespace a9n::kernel
{
    namespace
    {
        inline constexpr capability_error
            convert_hal_to_capability_error([[maybe_unused]] a9n::hal::hal_error e)
        {
            return capability_error::FATAL;
        }

        inline constexpr capability_error convert_lookup_error(capability_lookup_error e)
        {
            return capability_error::INVALID_DESCRIPTOR;
        }
    }

    /*
    capability_result interrupt_region::execute(process &owner, capability_slot &self)
    {
        auto operation = static_cast<operation_type>(
            a9n::hal::get_message_register(owner, OPERATION_TYPE)
                .unwrap_or(static_cast<a9n::word>(OPERATION_NONE))
        );

        switch (operation)
        {
            case OPERATION_MAKE_PORT :
                return operation_make_port(owner, self);

            default :
                return capability_error::ILLEGAL_OPERATION;
        }
    }

    capability_result interrupt_region::operation_make_port(process &owner, capability_slot &self)
    {
        // 1) TARGET_NODE(Descriptor) を取得
        return a9n::hal::get_message_register(owner, TARGET_NODE)
            .transform_error(convert_hal_to_capability_error)
            .and_then(
                [&](a9n::word node_descriptor) -> capability_result
                {
                    // 2) traverse_slot して node_slot を取得
                    return owner.root_slot.component
                        ->traverse_slot(node_descriptor, extract_depth(node_descriptor),
a9n::BYTE_BITS) .transform_error(convert_lookup_error) .and_then(
                            [&](capability_slot *node_slot) -> capability_result
                            {
                                if (node_slot->type != capability_type::NODE)
                                {
                                    return capability_error::INVALID_DESCRIPTOR;
                                }

                                // 3) TARGET_NODE_INDEX, IRQ_NUMBER を取得
                                return a9n::hal::get_message_register(owner, TARGET_NODE_INDEX)
                                    .transform_error(convert_hal_to_capability_error)
                                    .and_then(
                                        [&](a9n::word tindex) -> capability_result
                                        {
                                            return a9n::hal::get_message_register(owner, IRQ_NUMBER)
                                                .transform_error(convert_hal_to_capability_error)
                                                .and_then(
                                                    [&](a9n::word irq) -> capability_result
                                                    {
                                                        // 4) InterruptPort インスタンスを作成
                                                        auto *new_port = new (std::nothrow)
                                                            interrupt_port(irq);
                                                        if (!new_port)
                                                        {
                                                            return capability_error::OUT_OF_MEMORY;
                                                        }

                                                        // 5) node_ptr->slots[tindex]
                                                        // が範囲内かチェック
                                                        if (tindex >= node_ptr->max_slots)
                                                        {
                                                            delete new_port;
                                                            return capability_error::INVALID_INDEX;
                                                        }

                                                        auto &target_slot = node_ptr->slots[tindex];

                                                        // 6) 既に使用中かどうか
                                                        if (target_slot.type !=
capability_type::UNUSED)
                                                        {
                                                            delete new_port;
                                                            return capability_error::SLOT_IN_USE;
                                                        }

                                                        // 7) slotをInterrupt Portとして初期化
                                                        auto cfg_res =
try_configure_interrupt_port_slot( target_slot, *new_port, irq
                                                        );
                                                        if (!cfg_res)
                                                        {
                                                            // kernel_error -> capability_error 変換
                                                            return
convert_kernel_to_capability_error( cfg_res.error()
                                                            );
                                                        }

                                                        // 正常に生成完了
                                                        return {};
                                                    }
                                                );
                                        }
                                    );
                            }
                        );
                }
            );
    }
*/
}
