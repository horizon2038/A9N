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
