#ifndef LIBA9N_RESULT_HPP
#define LIBA9N_RESULT_HPP

#include <liba9n/result/__result/result_common.hpp>
#include <liba9n/result/__result/result_impl.hpp>
#include <liba9n/result/__result/result_void_impl.hpp>

#define TRY(expr) \
    ({ \
        auto &&__result = (expr); \
        if (__result.is_error()) \
        { \
            return __result.unwrap_error(); \
        } \
        __result.unwrap(); \
    })

#define TRY_VOID(expr) \
    ({ \
        auto &&__result = (expr); \
        if (__result.is_error()) \
        { \
            return __result.unwrap_error(); \
        } \
    })

#endif
