# x86_64 ABI Conformance Example

This payload verifies the x86_64 boundary between an A9N user image and the kernel without Nun or `a9n_abi`. It is a conformance test, not an application or system-init template.

## Prerequisites

Complete the manual's Getting Started procedure first. Use the same SPENCER checkout and its A9N submodule. Do not clone A9N separately.

The commands below run from the SPENCER directory.

## Build

```sh
ABI_EXAMPLE=A9N/doc/a9n-manual/examples/x86_64-hello

cargo xtask build \
  --arch x86-64 \
  --platform qemu \
  --release \
  --os-manifest "$ABI_EXAMPLE/Cargo.toml" \
  --os-target-json "$ABI_EXAMPLE/x86_64-unknown-a9n.json" \
  --os-binary a9n-manual-hello
```

SPENCER places the payload at `out/x86_64-qemu-release/nun_os_target_dir/x86_64-unknown-a9n/release/a9n-manual-hello` and stores it as `/kernel/init.elf` in the disk image.

## Inspect the ELF layout

```sh
INIT_ROOT=out/x86_64-qemu-release/nun_os_target_dir
INIT_ELF="$INIT_ROOT/x86_64-unknown-a9n/release/a9n-manual-hello"
llvm-readelf -h -l -s "$INIT_ELF"
```

Verify the following values:

- ELF type: `EXEC`; machine: `Advanced Micro Devices X86-64`
- Entry point: `0x1000`
- `__init_info_start`: `0x3000`
- `__init_ipc_buffer_start`: `0x4000`
- `__init_stack_end`: `0x15000`
- IPC buffer alignment: `0x4000 mod 0x1000 = 0`

The loader reads `__init_info_start` and `__init_ipc_buffer_start` from the ELF symbol table. Do not strip the symbol table. The load segments must cover the stack end.

## Run

```sh
ABI_EXAMPLE=A9N/doc/a9n-manual/examples/x86_64-hello

cargo xtask run \
  --arch x86-64 \
  --platform qemu \
  --release \
  --os-manifest "$ABI_EXAMPLE/Cargo.toml" \
  --os-target-json "$ABI_EXAMPLE/x86_64-unknown-a9n.json" \
  --os-binary a9n-manual-hello
```

A successful run prints:

```text
A9N manual ABI check: OK
kernel version: 0x0.1.10
capability call check: OK
capability error check: OK
```

The capability-call check reads the PCB capability in root slot 1 through descriptor `0x0801_0000_0000_0000`. It also verifies that `MR10` is backed by the IPC buffer. The capability-error check supplies descriptor zero and expects `MR0 = 0` with `MR1 = INVALID_DESCRIPTOR`.

The exact kernel version string depends on the A9N revision selected by SPENCER.
