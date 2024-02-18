#ifndef GENERIC_OPERATION_HPP
#define GENERIC_OPERATION_HPP

#include <library/common/types.hpp>

namespace library::capability
{
    // generic.execute(CONVERT, type, size, descriptor, descriptor_depth,
    // descriptor_index);
    enum class generic_operation : common::word
    {
        CONVERT,
    };
}

#endif
