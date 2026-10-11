# Physical page zero regression tests

From the A9N repository root:

```sh
sh test/host/run-frame-zero.sh
SANITIZE=1 sh test/host/run-frame-zero.sh
```

Both runs compile and execute the production x86_64 and AArch64 memory managers
on the host, plus production Generic allocation arithmetic and Frame capability
configuration. Only the physical-to-host pointer translation, privileged register
access/TLB effects, and unused logging/message-register effects are substituted.
The production PTE layouts, page-table traversal and mapping operations are used.

Coverage includes a 4 GiB Generic starting at PA 0, normal/device Frame metadata,
4 KiB/2 MiB/1 GiB mappings of PA 0 at a nonzero VA, unmap/remap, TLB invalidation,
read-only/non-executable permissions, and rejection of invalid sizes, alignment
and duplicate mappings. No host memory at address zero is accessed.

These tests do not replace booting the kernel on actual hardware.

# External IRQ completion regression tests

```sh
sh test/host/run-external-irq.sh
SANITIZE=1 sh test/host/run-external-irq.sh
```

These compile the production `interrupt_manager.cpp` with x86_64 headers,
both with and without SMP. Process lookup, notification delivery and HAL
mask/EOI operations are substituted; the handler and its result/error chains
are not. Ten scenarios, each exercised for IRQ 0, 12, 16 and 23, verify exactly
one mask attempt followed by exactly one EOI attempt: success, missing
manager/current process, notification failure,
missing/invalid/null notification handler, invalid IRQ, mask failure and EOI
failure. In particular, a mask failure must not skip EOI.

Before the fix, lookup/notification failures returned without completing the
IRQ, and the successful path acknowledged before masking. The test reproduces
the omitted completion; the updated handler passes both configurations and
AddressSanitizer/UndefinedBehaviorSanitizer. These are error-injection tests,
not evidence that a particular physical-machine hang took that path. They do
not exercise real interrupt-controller behavior or the separate timer/IPI paths.

Compile-time checks also pin the CPU exception boundary and Local APIC timer
to vector `0x20`, the external IRQ base to `0x30`, and USB IRQ 16 to `0x40`.
IRQ 23 must remain below the first reserved IPI vector in both SMP builds.
The IO APIC still applies MADT overrides independently (e.g. IRQ 0 uses GSI 2
but vector `0x30`); runtime routing and timer delivery require a boot test.

# Timer IRQ completion regression tests

```sh
sh test/host/run-timer-irq.sh
SANITIZE=1 sh test/host/run-timer-irq.sh
```

These compile the production `interrupt_manager.cpp` both with and without SMP.
Process-manager lookup, timer processing and HAL EOI are substituted. All eight
combinations of success/failure at those three stages verify exactly one EOI
attempt after timer processing, including lookup/processing failure and EOI
failure itself. Timer processing must not run if manager lookup failed.

Before the fix, the error cases skipped EOI because it was inside the success
chain. The corrected handler completes the IRQ outside that chain while still
holding the giant lock. These tests do not exercise hardware IRQ delivery or
establish that the physical-machine stall encountered an injected error.

# Local APIC timer start-order regression

```sh
sh test/host/run-local-apic-timer.sh
SANITIZE=1 sh test/host/run-local-apic-timer.sh
```

These compile the production `local_apic_timer.cpp`, substituting only APIC
register access, the calibration wait and logging. The counter model lets time
elapse immediately after the initial-count write, enough to expire a one-shot
counter before the next register write. The old BSP/AP initialization returns
success with a stopped counter in that case. The corrected order is divide,
periodic LVT (vector `0x20`), then initial count, and keeps counting periodically.

Coverage includes both entry points with and without the injected delay,
uninitialized/zero-frequency rejection, and errors at each programming write.
Failed setup must not replace the frequency used to start subsequent APs.
The test does not simulate interrupt delivery or prove that the physical-machine
stall encountered this delay; a hardware boot test is still required.

# Direct-schedule quantum regression tests

```sh
sh test/host/run-direct-schedule-quantum.sh
SANITIZE=1 sh test/host/run-direct-schedule-quantum.sh
```

These compile the production process manager and scheduler with and without SMP.
Only HAL local-variable lookup, context switching and logging are substituted.
Nine scenarios cover repeated self-switches, switches to another process,
a self-switch request selecting a queued higher-priority process, timer expiry
despite repeated self-switches, an invalid target rejected without altering
quantum, repeated caller/server switches, continuous calls between server timer
ticks, continuous calls between caller timer ticks, and a target whose quantum
is already exhausted. Both direct-schedule
overloads preserve each process's remaining quantum.

Quantum donation is removed, including the former `ShouldAddQuantum` template
parameter. Before removal, switching to another process increased its quantum;
the unchanged-quantum regression fails against that implementation. After 1,000
modeled calls, a queued peer runs within the server's original five remaining
ticks. Continuous calls between server timer ticks must also leave that quantum
unchanged, allowing a same-priority third process to run when it expires.
Direct scheduling, priority selection, cross-core routing and interrupt vectors
are unchanged. Fast Reply-Receive no longer refreshes the caller's quantum;
modeled continuous calls must therefore let a queued peer run when the caller's
seven remaining ticks expire, even if all timer ticks fall on that caller.
These scheduler host tests do not execute the full IPC Reply path or physical SMP races,
and do not establish that quantum growth caused the reported hardware stall.

# Receive notifications and Reply-Receive quantum

```sh
sh test/host/run-pending-receive.sh
SANITIZE=1 sh test/host/run-pending-receive.sh
```

These tests compile the actual IPC and notification capability code, with
scheduling, HAL register access and logging substituted. They invoke the
capability's public `execute` entry point, both with and without SMP.
Before the fix, Reply-Receive consumed a bound pending notification without
blocking, but plain Receive left it pending and blocked the server. Plain
Receive now uses the same pending-notification helper before entering the
receive queue. Pending-notification handling in Call and Reply-Receive is unchanged.

Thirteen scenarios cover blocking/nonblocking Receive and the Reply-Receive control,
subsequent blocking and notification wakeup, another queued receiver, precedence
of an already queued sender, empty nonblocking Receive, removal of stale reply
links, injected MR2/MR3 delivery errors, and missing read permission. They also
check that a notification return does not advertise a receiver that was never
queued. Three additional scenarios perform 1,000 actual Call/Reply-Receive
transactions each, with scheduling substituted: a partially consumed caller
quantum, an exhausted caller quantum, and a same-core fast return on core 2
(BSP in the non-SMP build). They verify preserved budgets, payload transfer,
reply-link cleanup and the server's receive-queue state. Before removing the
fast Reply-Receive reset, the first transaction changes the caller's budget
and fails this regression. Both SMP configurations pass with and without
AddressSanitizer and UndefinedBehaviorSanitizer.

Unlike the SDK deferred-notification regression, this flag has **not** been
consumed by a previous Call: it is still in the kernel notification object.
If it represents an already-masked timer IRQ, the driver may block waiting for
another event while the IRQ needed to wake it cannot be delivered. Whether the
reported physical-machine halt took this path remains unconfirmed.
