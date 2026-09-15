#include <hal/interface/memory_manager.hpp>
#include <kernel/capability/frame_capability.hpp>
#include <kernel/capability/generic.hpp>
#include <kernel/utility/logger.hpp>
#include <test_machine.hpp>
#include <stdio.h>
#include <stdlib.h>

#ifdef TEST_AARCH64
#include <hal/aarch64/memory/paging.hpp>
#else
#include <hal/x86_64/memory/paging.hpp>
#endif

using namespace a9n::kernel;

#define CHECK(expression)                                                        \
    do {                                                                        \
        if (!(expression)) {                                                    \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression);    \
            abort();                                                            \
        }                                                                       \
    } while (0)

namespace a9n::kernel::utility
{
    void logger::error(const char *) { abort(); }
    void logger::printk(const char *, ...) {}
    void logger::printh(const char *, ...) {}
}
namespace a9n::hal
{
    hal_result configure_message_register(process &, a9n::word, a9n::word) { return {}; }
}

static void capability_tests()
{
    // Allocation arithmetic and frame configuration use production kernel code.
    // Cover the user's example: a 4GiB Generic beginning at physical address 0.
    generic_info region { 0, 32, false, 0 };
    CHECK(region.is_allocatable(30, 4));
    for (a9n::word i = 0; i < 4; ++i)
    {
        auto allocated = region.try_apply_allocate(30);
        CHECK(allocated);
        CHECK(allocated.unwrap() == (i << 30));
        capability_slot slot {};
        frame target { allocated.unwrap(), 30, 0 };
        CHECK(try_configure_frame_slot(slot, target));
        CHECK(slot.type == capability_type::FRAME);
        CHECK(slot.component == &frame_capability_core);
        CHECK(slot.rights == capability_slot::object_rights::ALL);
        auto decoded = convert_slot_data_to_frame(slot.data);
        CHECK(decoded.address == target.address && decoded.size_bits == 30);
    }
    CHECK(region.current_watermark() == (1ULL << 32));
    CHECK(!region.try_apply_allocate(12));
    const a9n::word flags_list[] { 0, FRAME_FLAG_DEVICE };
    for (auto flags : flags_list)
    {
        capability_slot slot {};
        CHECK(try_configure_frame_slot(slot, frame { 0, 12, flags }));
        CHECK(convert_slot_data_to_frame(slot.data).flags == flags);
    }
}

static void mapping_test(a9n::word size_bits, bool device)
{
    alignas(4096) a9n::word tables[4][512] {};
    const page_table root { reinterpret_cast<a9n::physical_address>(tables[0]), 4 };
    constexpr a9n::virtual_address va = 1ULL << 30; // PA 0 need not map at VA 0.
    const a9n::word leaf_depth = size_bits == 12 ? 1 : size_bits == 21 ? 2 : 3;
    test_machine::active_root = root.address;
    const auto rights = static_cast<a9n::word>(rights::READ);
    for (a9n::word depth = 3; depth >= leaf_depth; --depth)
    {
        page_table child { reinterpret_cast<a9n::physical_address>(tables[4 - depth]), depth };
        CHECK(a9n::hal::map_page_table(root, child, va, rights));
    }
    const frame zero { 0, size_bits, device ? FRAME_FLAG_DEVICE : 0 };
    const auto before = test_machine::tlb_invalidations;
    CHECK(a9n::hal::map_frame(root, zero, va, rights));
    CHECK(test_machine::tlb_invalidations == before + 1);
    const auto leaf_index = (va >> (12 + 9 * (leaf_depth - 1))) & 511;
    auto &leaf = tables[4 - leaf_depth][leaf_index];
#ifdef TEST_AARCH64
    namespace paging = a9n::hal::aarch64;
    CHECK((leaf & paging::descriptor_address_mask) == 0);
    CHECK(leaf & paging::descriptor_valid);
    CHECK(leaf & paging::descriptor_read_only);
    CHECK(leaf & paging::descriptor_uxn);
    CHECK((leaf & (7ULL << 2)) == (device ? paging::descriptor_attr_device : paging::descriptor_attr_normal));
#else
    a9n::hal::x86_64::page pte { .all = leaf };
    CHECK(pte.get_physical_address() == 0 && pte.present);
    CHECK(!pte.rw && pte.execute_disable && pte.user_supervisor);
    CHECK(pte.page_size == (size_bits != 12));
#endif
    CHECK(!a9n::hal::map_frame(root, zero, va, rights)); // Existing mapping remains protected.
    CHECK(a9n::hal::unmap_frame(root, zero, va));
    CHECK(leaf == 0);
    CHECK(test_machine::tlb_invalidations == before + 2);
    CHECK(!a9n::hal::map_frame(root, zero, va + 1, rights));
    CHECK(!a9n::hal::map_frame(root, frame { 1, size_bits, 0 }, va, rights));
    CHECK(!a9n::hal::map_frame(root, frame { 0, 13, 0 }, va, rights));
    CHECK(!a9n::hal::unmap_frame(root, frame { 0, 13, 0 }, va));
    CHECK(leaf == 0);
    CHECK(a9n::hal::map_frame(root, zero, va, rights)); // Remapping PA 0 succeeds.
    CHECK(a9n::hal::unmap_frame(root, zero, va));
}

int main()
{
    capability_tests();
    const a9n::word sizes[] { 12, 21, 30 };
    for (auto size : sizes)
    {
        mapping_test(size, false);
        mapping_test(size, true);
    }
    puts("PASS: PA 0 frame capabilities, 4GiB allocation, map/unmap, rights and invalid inputs");
}
