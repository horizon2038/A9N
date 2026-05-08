#ifndef A9N_KERNEL_LOCK_HPP
#define A9N_KERNEL_LOCK_HPP

#include <hal/hal_result.hpp>
#include <kernel/kernel_result.hpp>
#include <kernel/process/cpu.hpp>
#include <kernel/types.hpp>

#include <hal/interface/cpu.hpp>
#include <hal/interface/lock.hpp>

namespace a9n::kernel
{
    template<typename T>
    concept lockable = requires(T t) {
        { t.lock() };
        { t.unlock() };
    };

    class spin_lock
    {
      private:
        alignas(64) volatile uint8_t locked;
        alignas(64) a9n::sword owner_id { -1 };

      public:
        spin_lock(void)                              = default;
        spin_lock(const spin_lock &other)            = delete;
        spin_lock(spin_lock &&other)                 = delete;

        spin_lock &operator=(const spin_lock &other) = delete;
        spin_lock &operator=(spin_lock &&other)      = delete;

        kernel_result try_lock(void)
        {
            uint8_t old = a9n::hal::atomic_exchange(&locked, 1);
            if (old != 0)
            {
                return kernel_error::TRY_AGAIN;
            }

            return a9n::hal::current_local_variable()
                .and_then(
                    [&](cpu_local_variable *variable) -> a9n::hal::hal_result
                    {
                        owner_id = variable->core_number;
                        return {};
                    }
                )
                .transform_error(convert_hal_to_kernel_error);
        }

        kernel_result lock(void)
        {
            for (;;)
            {
                auto result = try_lock();
                if (result)
                {
                    break;
                }
            }

            return {};
        }

        kernel_result unlock(void)
        {
            return a9n::hal::current_local_variable()
                .and_then(
                    [&](cpu_local_variable *variable) -> a9n::hal::hal_result
                    {
                        if (variable->core_number != owner_id)
                        {
                            return hal::hal_error::TRY_AGAIN;
                        }

                        owner_id = -1;
                        a9n::hal::atomic_exchange(&locked, 0);

                        return {};
                    }
                )
                .transform_error(convert_hal_to_kernel_error);
        }
    };

    class spin_lock_no_owner
    {
      private:
        alignas(64) volatile uint8_t locked { 0 };

      public:
        spin_lock_no_owner(void)                                       = default;
        spin_lock_no_owner(const spin_lock_no_owner &other)            = delete;
        spin_lock_no_owner(spin_lock_no_owner &&other)                 = delete;

        spin_lock_no_owner &operator=(const spin_lock_no_owner &other) = delete;
        spin_lock_no_owner &operator=(spin_lock_no_owner &&other)      = delete;

        kernel_result try_lock(void)
        {
            uint8_t old = a9n::hal::atomic_exchange(&locked, 1);
            if (old != 0)
            {
                return kernel_error::TRY_AGAIN;
            }

            return {};
        }

        kernel_result lock(void)
        {
            for (;;)
            {
                auto result = try_lock();
                if (result)
                {
                    break;
                }
            }

            return {};
        }

        kernel_result unlock(void)
        {
            uint8_t old = a9n::hal::atomic_exchange(&locked, 0);

            return {};
        }
    };

    template<lockable Lock>
    class lock_guard
    {
      public:
        lock_guard(Lock &target_lock) : lock { target_lock }
        {
            lock.lock();
        }

        ~lock_guard()
        {
            lock.unlock();
        }

        lock_guard(const lock_guard &guard)            = delete;
        lock_guard(lock_guard &&guard)                 = delete;

        lock_guard &operator=(const lock_guard &other) = delete;
        lock_guard &operator=(lock_guard &&other)      = delete;

      private:
        Lock &lock;
    };

    template<lockable Lock, typename Function>
        requires(liba9n::std::is_same_v<
                 liba9n::std::remove_cvref_t<liba9n::std::invoke_result_t<Function>>,
                 kernel_result>)
    kernel_result lock_region(Lock &lock, Function function)
    {
        return lock.lock()
            .and_then(
                [&](void) -> kernel_result
                {
                    return function();
                }
            )
            .and_then(
                [&](void) -> kernel_result
                {
                    return lock.unlock();
                }
            );
    }

    // kernel giant lock
    inline spin_lock giant_lock {};
}

#endif
