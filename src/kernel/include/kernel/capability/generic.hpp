#ifndef GENERIC_HPP
#define GENERIC_HPP

#include <kernel/capability/capability_local_state.hpp>
#include <kernel/capability/capability_component.hpp>
#include <kernel/ipc/message_buffer.hpp>

#include <library/common/types.hpp>

namespace kernel
{
    struct memory_region
    {
        common::physical_address base_address;
        common::physical_address end_address;
    };

    // clang-format off
    // auto size = factory.calculate_memory_size(common::word type, common::word size_bits);
    // auto free_region = create_free_region(const generic_info &info, common::word size);
    // if (info.is_device)
    // {
    //     return;
    // }
    // if (!is_allocatable(free_region, size))
    // {
    //     // error : illegal size (out of range)
    //     return -1;
    // }
    // auto slot = retrieve_target_slot(root_slot, buffer);
    // *slot = factory.make(common::word type, free_region);
    // 
    // auto new_info = update_generic_info(const &generic_info info, free_region, size);
    //
    // メモリサイズを取得する
    // 空き領域オブジェクトを作成する
    // 空き領域にメモリサイズが収まっているかを判定する
    // 空き領域にCapabilityを配置する
    // 空き領域とサイズから新しいdataを作る
    // 反映させる
    //
    // struct memory_region;
    // struct capability_info
    // {
    //     capability_type type;
    //     common::word size_bits;
    //     common::word memory_size;
    // }
    //
    // auto this_info = create_generic_info(const capability_slot_data &data);
    // capability_factory factory {};
    //
    // auto memory_size = factory.calculate_memory_size(capability_type type, common::word size_bits);
    // auto free_region = create_free_region(const &generic_info info);
    // auto aligned_free_region = align_region(const memory_region &free_region, common::word memory_size);
    //
    // auto target_slot_location = create_capability_location(const message_buffer &message_buffer);
    // auto target_slot = retrieve_target_slot(const capability_location &target_slot_location);
    // target_slot = factory.make(capability_type type, common::word size_bits, const memory_region &taget_region);
    //
    // this_slot = update_after_allocation(aligned_free_region, common::word memory_size);
    //
    // clang-format on

    class generic_flags
    {
      public:
        const common::word value;

        generic_flags(common::word target_value) : value(target_value)
        {
        }

        inline bool is_device() const
        {
            return (value >> 7) & 1;
        }

        inline common::word size_bits() const
        {
            common::word size_mask = (1 << (7)) - 1;
            return value & size_mask;
        }

        inline common::word size() const
        {
            return static_cast<common::word>(1) << size_bits();
        }
    };

    inline generic_flags
        create_generic_flags(bool is_device, library::common::word size_bits)
    {
        library::common::word value {};

        value |= (is_device << 7);
        auto size_bits_mask = (1 << 7) - 1;
        value |= (size_bits & size_bits_mask);

        return generic_flags { value };
    }

    struct generic_info
    {
        common::physical_address base_address;
        generic_flags flags;
        common::physical_address watermark;
    };

    class generic final : public capability_component
    {
      public:
        common::error execute(
            capability_slot *this_slot,
            capability_slot *root_slot,
            message_buffer *buffer
        ) override;

        common::error revoke() override;

        // empty implements (for composite-pattern)
        capability_slot *retrieve_slot(common::word index) override
        {
            return nullptr;
        }

        capability_slot *traverse_slot(
            library::capability::capability_descriptor descriptor,
            common::word max_bits,
            common::word used_bits
        ) override
        {
            return nullptr;
        };

      private:
        common::error decode_operation(
            capability_slot *this_slot,
            capability_slot *root_slot,
            message_buffer *buffer
        );

        common::error convert(
            capability_slot *this_slot,
            capability_slot *root_slot,
            message_buffer *buffer
        );

        common::error create_generic(
            capability_slot *this_slot,
            capability_slot *root_slot,
            message_buffer *buffer
        );

        generic_info create_generic_info(capability_slot_data *data);

        capability_slot *retrieve_target_slot(
            capability_slot *root_slot,
            message_buffer *buffer
        );

        // create dest capability_slot

        generic_info create_child_generic_info(
            generic_info *parent_info,
            message_buffer *buffer
        );

        bool is_allocatable(generic_info *parent_info, common::word size);

        capability_slot_data create_generic_slot_data(generic_info *info);

        generic_info update_parent_generic_info(
            generic_info *parent_info,
            generic_info *child_info
        );
    };

}

#endif
