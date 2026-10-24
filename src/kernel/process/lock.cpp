#include <kernel/process/lock.hpp>
#include <kernel/process/reclamation.hpp>

#include <hal/interface/lock.hpp>

namespace a9n::kernel
{
    void spin_lock::lock_contended(kernel_entry_type entry)
    {
        do
        {
            a9n::hal::spin_wait();
            if constexpr (SMP_ENABLED)
            {
                // Only giant-lock waiters may retire their current context.
                if (static_cast<const void *>(this) == static_cast<const void *>(&giant_lock))
                {
                    service_pending_stop_request(entry);
                }
            }
        } while (!try_lock());
    }
}
