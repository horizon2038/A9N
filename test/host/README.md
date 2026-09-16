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
are not. Ten scenarios verify exactly one mask attempt followed by exactly
one EOI attempt: success, missing manager/current process, notification failure,
missing/invalid/null notification handler, invalid IRQ, mask failure and EOI
failure. In particular, a mask failure must not skip EOI.

Before the fix, lookup/notification failures returned without completing the
IRQ, and the successful path acknowledged before masking. The test reproduces
the omitted completion; the updated handler passes both configurations and
AddressSanitizer/UndefinedBehaviorSanitizer. These are error-injection tests,
not evidence that a particular physical-machine hang took that path. They do
not exercise real interrupt-controller behavior or the separate timer/IPI paths.

# Pending notification at plain Receive

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
receive queue. Call and Reply-Receive paths are unchanged.

Ten scenarios cover blocking/nonblocking Receive and the Reply-Receive control,
subsequent blocking and notification wakeup, another queued receiver, precedence
of an already queued sender, empty nonblocking Receive, removal of stale reply
links, injected MR2/MR3 delivery errors, and missing read permission. They also
check that a notification return does not advertise a receiver that was never
queued. Both SMP configurations pass with and without AddressSanitizer and
UndefinedBehaviorSanitizer.

Unlike the SDK deferred-notification regression, this flag has **not** been
consumed by a previous Call: it is still in the kernel notification object.
If it represents an already-masked timer IRQ, the driver may block waiting for
another event while the IRQ needed to wake it cannot be delivered. Whether the
reported physical-machine halt took this path remains unconfirmed.
