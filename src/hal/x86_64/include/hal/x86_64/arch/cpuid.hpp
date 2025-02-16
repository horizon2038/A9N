#ifndef A9N_HAL_X86_64_CPUID_HPP
#define A9N_HAL_X86_64_CPUID_HPP

#include <hal/hal_result.hpp>
#include <kernel/types.hpp>

#include <liba9n/common/enum.hpp>
#include <liba9n/libc/string.hpp>
#include <liba9n/libcxx/array>

#include <kernel/utility/logger.hpp>

namespace a9n::hal::x86_64
{
    // eax value list
    // cf., Intel SDM vol. 2A 3-217, table 3-8, "CPUID - CPU Identification"
    //      (combined : page 814)
    // naming : cf., https://en.wikipedia.org/wiki/CPUID
    enum class cpuid_leaf : uint32_t
    {
        // basic cpuid information
        HIGHEST_FUNCTION_PARAMETER_AND_MANUFACTURER_ID = 0x00000000,
        PROCESSOR_INFO_AND_FEATURE_BITS                = 0x00000001,
        CACHE_AND_TLB_INFORMATION                      = 0x00000002,
        PROCESSOR_SERIAL_NUMBER                        = 0x00000003,

        // structured extended feature flags enumeration leaf
        EXTENDED_FEATURES = 0x00000007,

        // processor extended state enumeration main leaf
        XSAVE_FEATURES_AND_STATE_COMPONENTS = 0x0000000d,

        // extended function cpuid information
        HIGHEST_EXTENDED_FUNCTION_IMPLEMENTED    = 0x80000000,
        EXTENDED_PROCESSOR_INFO_AND_FEATURE_BITS = 0x80000001,
        PROCESSOR_BRAND_STRING_0                 = 0x80000002,
        PROCESSOR_BRAND_STRING_1                 = 0x80000003,
        PROCESSOR_BRAND_STRING_2                 = 0x80000004,
    };

    namespace vendor_identifier
    {
        inline constexpr const char *INTEL = "GenuineIntel";
        inline constexpr const char *AMD   = "AuthenticAMD";
    }

    // feature information bits (ECX) : PROCESSOR_INFO_AND_FEATURE_BITS
    // cf., Intel SDM vol. 2A 3-243, table 3-(7|10), "Feature Information Returned in the ECX
    //      Register"
    //      (combined : page 838)
    enum class cpuid_feature_information : uint32_t
    {
        SSE3                = static_cast<uint32_t>(1) << 0,
        PCLMULQDQ           = static_cast<uint32_t>(1) << 1,
        DTES64              = static_cast<uint32_t>(1) << 2,
        MONITOR             = static_cast<uint32_t>(1) << 3,
        DS_CPL              = static_cast<uint32_t>(1) << 4,
        VMX                 = static_cast<uint32_t>(1) << 5,
        SMX                 = static_cast<uint32_t>(1) << 6,
        EIST                = static_cast<uint32_t>(1) << 7,
        TM2                 = static_cast<uint32_t>(1) << 8,
        SSSE3               = static_cast<uint32_t>(1) << 9,
        CNXT_ID             = static_cast<uint32_t>(1) << 10,
        SDBG                = static_cast<uint32_t>(1) << 11,
        FMA                 = static_cast<uint32_t>(1) << 12,
        CMPXCHG16B          = static_cast<uint32_t>(1) << 13,
        XTPR_UPDATE_CONTORL = static_cast<uint32_t>(1) << 14,
        PDCM                = static_cast<uint32_t>(1) << 15,
        RESERVED_0          = static_cast<uint32_t>(1) << 16,
        PCID                = static_cast<uint32_t>(1) << 17,
        DCA                 = static_cast<uint32_t>(1) << 18,
        SSE4_1              = static_cast<uint32_t>(1) << 19,
        SSE4_2              = static_cast<uint32_t>(1) << 20,
        X2APIC              = static_cast<uint32_t>(1) << 21,
        MOVBE               = static_cast<uint32_t>(1) << 22,
        POPCNT              = static_cast<uint32_t>(1) << 23,
        TSC_DEADLINE        = static_cast<uint32_t>(1) << 24,
        AES                 = static_cast<uint32_t>(1) << 25,
        XSAVE               = static_cast<uint32_t>(1) << 26,
        OSXSAVE             = static_cast<uint32_t>(1) << 27,
        AVX                 = static_cast<uint32_t>(1) << 28,
        F16C                = static_cast<uint32_t>(1) << 29,
        RORAND              = static_cast<uint32_t>(1) << 30,
        RESERVED_1          = static_cast<uint32_t>(1) << 31,
    };

    struct cpuid_info
    {
        uint32_t rax;
        uint32_t rbx;
        uint32_t rcx;
        uint32_t rdx;
    } __attribute__((packed));

    extern "C" void _cpuid(uint32_t leaf, uint32_t subleaf, cpuid_info *info);

    inline liba9n::result<cpuid_info, hal_error> try_get_cpuid(cpuid_leaf leaf, uint32_t subleaf)
    {
        cpuid_info info;

        _cpuid(liba9n::enum_cast(leaf), subleaf, &info);

        return info;
    }

    // sizeof(uint32_t) * 4 + 1
    using vendor_id = liba9n::std::array<char, 13>;

    inline liba9n::result<vendor_id, hal_error> try_get_vendor_id(void)
    {
        return try_get_cpuid(cpuid_leaf::HIGHEST_FUNCTION_PARAMETER_AND_MANUFACTURER_ID, 0)
            .and_then(
                [](cpuid_info info) -> liba9n::result<vendor_id, hal_error>
                {
                    vendor_id id;

                    liba9n::std::memcpy(reinterpret_cast<char *>(id.data()) + 0x00, &info.rbx, 4);
                    liba9n::std::memcpy(reinterpret_cast<char *>(id.data()) + 0x04, &info.rdx, 4);
                    liba9n::std::memcpy(reinterpret_cast<char *>(id.data()) + 0x08, &info.rcx, 4);
                    id.data()[12] = '\0';

                    return id;
                }
            );
    }

    // sizeof(uint32_t) * 4 * 3 +1
    using cpu_name = liba9n::std::array<char, 49>;

    inline liba9n::result<cpu_name, hal_error> try_get_cpu_name(void)
    {
        cpu_name name;

        return try_get_cpuid(cpuid_leaf::PROCESSOR_BRAND_STRING_0, 0)
            .and_then(
                [&](cpuid_info info) -> hal_result
                {
                    liba9n::std::memcpy(
                        reinterpret_cast<char *>(name.data()) + sizeof(cpuid_info) * 0,
                        &info,
                        sizeof(cpuid_info)
                    );

                    return {};
                }
            )
            .and_then(
                [&](void) -> liba9n::result<cpuid_info, hal_error>
                {
                    return try_get_cpuid(cpuid_leaf::PROCESSOR_BRAND_STRING_1, 0);
                }
            )
            .and_then(
                [&](cpuid_info info) -> hal_result
                {
                    liba9n::std::memcpy(
                        reinterpret_cast<char *>(name.data()) + sizeof(cpuid_info) * 1,
                        &info,
                        sizeof(cpuid_info)
                    );

                    return {};
                }
            )
            .and_then(
                [&](void) -> liba9n::result<cpuid_info, hal_error>
                {
                    return try_get_cpuid(cpuid_leaf::PROCESSOR_BRAND_STRING_2, 0);
                }
            )
            .and_then(
                [&](cpuid_info info) -> hal_result
                {
                    liba9n::std::memcpy(
                        reinterpret_cast<char *>(name.data()) + sizeof(cpuid_info) * 2,
                        &info,
                        sizeof(cpuid_info)
                    );

                    return {};
                }
            )
            .and_then(
                [&](void) -> liba9n::result<cpu_name, hal_error>
                {
                    name.data()[48] = '\0';
                    return name;
                }
            );
    }

    inline liba9n::result<cpuid_info, hal_error> try_get_processor_info_and_feature(void)
    {
        return try_get_cpuid(cpuid_leaf::PROCESSOR_INFO_AND_FEATURE_BITS, 0);
    }
}

#endif
