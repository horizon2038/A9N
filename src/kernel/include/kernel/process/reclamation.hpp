#ifndef A9N_KERNEL_PROCESS_RECLAMATION_HPP
#define A9N_KERNEL_PROCESS_RECLAMATION_HPP

#include <kernel/kernel_result.hpp>
#include <kernel/capability/capability_result.hpp>

namespace a9n::kernel
{
    class process;

    // Contended giant-lock hook: service only this CPU's pending stop request.
    // The waiting CPU does not own the lock; the requester holds it until ACK.
    void service_pending_stop_request(void);

    // Synchronous stop shared by SUSPEND and revoke. The caller holds giant_lock
    // until the remote CPU acknowledges the context/page-table switch.
    kernel_result stop_process_for_reclamation(process &target);
    capability_result complete_revoked_wait(process &target);
}

#endif
