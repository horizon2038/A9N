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

# Capability reclamation and lookup hot-path regressions

```sh
sh test/host/run-capability-reclamation.sh
sh test/host/run-smp-reclamation.sh
sh test/host/run-capability-lookup-codegen.sh
```

The reclamation tests use the production Node implementation with AddressSanitizer
and UndefinedBehaviorSanitizer. They cover sibling aliases, slot-local identity,
weak-capability removal, Node containment/cycles, failed cleanup and retry, and
Node construction/descriptor traversal. Active Node finalizers are tracked
on a giant-lock-protected stack chain; lookup fields and the 32-byte Node layout
are unchanged. An internal lookup during a failing or successful finalizer
verifies that reclamation state does not alter lookup geometry.

Dependency traversal and Slot removal are implemented by the Node layer in
`capability_node_reclamation.cpp`; the existing Slot methods only forward to it.
Each Object's `revoke` owns its own teardown. There is no `prepare_revoke` hook or
global process-preparation pass. Descendants are reclaimed independently before
their parent callback, so failed cleanup prevents the parent Generic's allocator
reset. Tests check this postorder and retained dependency links on failure.
Slot size is also constrained to eight words by a production `static_assert`.

`last_alias_in_group` names the shared alias-boundary walk used by child lookup,
insertion and revocation. Slot removal spells out active-cycle, empty-Slot,
remaining-alias and final-alias cases, sharing only the unlink/reset step.
Regressions also exercise both const/mutable alias lookup and empty linked-Slot
removal without disturbing neighboring Slots or finalizing their Objects.

Node geometry checks run at construction/configuration. `retrieve_slot` retains
its original contract: callers supply an index within the Node's radix; it checks
Slot storage availability, not index bounds. `lookup_slot` extracts a radix-masked
index and calls `retrieve_slot`. `traverse_slot` retains its original `.and_then`
chain for termination and recursive lookup, without a per-Node reclamation flag
mask or new depth-validation branch.
Descriptors must supply a valid depth for each traversed Node, as in the
original lookup contract.

The code-generation check exports `BASELINE_REF` (default: `HEAD`) into an
isolated temporary directory, compiles baseline and current production Node
code for both architectures at O2/O3, retains their disassembly, and rejects
increases in static instruction or conditional-branch counts in `traverse_slot`
and `lookup_slot`. Like the production toolchains, its default compiler and
disassembler come from `llvm-config --bindir`; `CODEGEN_CXX` and `LLVM_OBJDUMP`
can override them.

These are translation-unit code-generation checks, not final-LTO byte equality
or real syscall latency measurements. In particular, the separate SMP
giant-lock contention/rendezvous is not measured by this lookup check.

`run-smp-reclamation.sh` exercises the production stop mailbox and giant lock
over 100 modeled IRQ-masked remote entries, checking acknowledgment after the
context switch, pending-target removal, and retry after a failed IPI send.
The existing spin lock uses a separate contention function, with no special
lock subclass, per-lock rendezvous flag, or nonstandard compiler attributes.
Only giant-lock waiters service stop requests; other locks cannot retire their
caller's context. Privileged HAL effects are substituted, and host atomic
operations use a pthread mutex. The test is not an IPC performance benchmark.
Request servicing and ACK waiting are named helpers in the contention/teardown
code; the ordinary lock-acquisition path is unchanged. IPI-send failure still
precedes request publication, and ACK still follows the context switch.
The stop protocol and its per-CPU requests live in `process/reclamation.cpp`;
`process/lock.cpp` only implements the contended lock loop and calls its
giant-lock reclamation hook.

A separate Release/full-LTO audit against the baseline above checked actual
x86_64 and AArch64 kernel executables. Uncontended kernel entry retains the
same register saves and has no additional lock checks, branches or calls;
the stop-request handling stays in the contention path. This is a code-path
comparison, not a claim of equal measured IPC latency.

# PCB configuration and reclamation regressions

```sh
sh test/host/run-pcb-lifecycle.sh
```

The production PCB implementation is tested with and without SMP under
AddressSanitizer/UndefinedBehaviorSanitizer; HAL and scheduling effects are
substituted. All 1,024 configuration masks are exercised across 21 profiles:
success, each message-read failure, invalid IP/SP/TLS, priority/affinity limits,
register-write failures, and retargeting a ready process. Five further cases
inject failure at each owned-slot cleanup stage and then retry, checking that
failed cleanup retains the backing metadata, including the
existing IPC-buffer clearing order. PCB `revoke` is called directly, without a
preparation call, and its owned-resource callbacks require the process to have
been stopped first. A further Node/PCB regression injects a stop failure during
ancestor reclamation: PCB resources, dependency
links and the ancestor allocation state must remain available for retry. The
current suite exercises 21,510 cases in each SMP configuration.

The PCB readability refactor keeps table-driven configuration, with typed,
named selection flags separate from message-register indices. The register
table explicitly preserves the distinct invalid-address error for TLS.
The suite pins the pre-refactor ordered-operation trace
`12865827607116404806` in both SMP configurations, covering argument reads,
configuration effects, errors and resource-cleanup order.

Before the Object-owned revoke change, both the pre-refactor and simplified
implementations produced 21,509 passing
cases and the same ordered-operation trace, 9504414185266343312. The trace covers
argument reads, capability lookup/binding, register writes, errors and cleanup.
Node tests additionally exercise COPY/MOVE/MINT through the public operation
entry, sibling depth/identity, weak rights and attenuation.

The simplification keeps IPC CALL/Reply-Receive and giant-lock entry code intact.
Release/full-LTO comparisons on both architectures found identical opcode
sequences and instruction/branch/call counts in IPC execute, Reply-Receive and
message-transfer helpers, and syscall entry. This is a code-generation check,
not a real-hardware latency measurement.

The Object-owned revoke refactor was separately checked against the preceding
simplified implementation. Release/full-LTO builds for x86_64 and AArch64 retain
the same opcode sequences and instruction/branch/call counts in the six audited
IPC/syscall-entry functions above. QEMU boots with the updated kernels passed
on both architectures with one and four CPUs, including Capability reclamation,
reuse and provider handover probes.

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

Port revoke cancels only senders and receivers still on that Port's queue.
Accepted CALLs remain blocked waiting for their receiver's reply, independently
of the original Port's lifetime. Additional regressions cover queue-first and
receiver-first acceptance, cancellation of queued calls/receivers, and destruction
and reconstruction of a Port at the same address while old and new CALLs wait.
Replies through a different live Port must reach the correct callers. PCB-side
reply cancellation still removes caller/server links when either process is
reclaimed. No global process registry or per-CALL Port reference is maintained.
Reply cancellation uses explicit `DETACH_ONLY` and `RESUME_WITH_ERROR` modes;
the former preserves both the suspended state and existing message registers.
The error-result register indices are named without changing their wire values.

Unlike the SDK deferred-notification regression, this flag has **not** been
consumed by a previous Call: it is still in the kernel notification object.
If it represents an already-masked timer IRQ, the driver may block waiting for
another event while the IRQ needed to wake it cannot be delivered. Whether the
reported physical-machine halt took this path remains unconfirmed.
