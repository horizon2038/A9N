#include <kernel/process/lock.hpp>
#include <kernel/process/reclamation.hpp>

#include <hal/interface/lock.hpp>

namespace a9n::kernel
{
    void spin_lock_no_owner::lock_contended(void)
    {
        do
        {
            a9n::hal::spin_wait();
            if constexpr (SMP_ENABLED)
            {
                // Only giant-lock waiters may retire their current context.
                if (static_cast<const void *>(this) == static_cast<const void *>(&giant_lock))
                {
                    service_pending_stop_request();
                }
            }
        } while (!try_lock());
    }
}
