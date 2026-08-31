#import "/components/reference.typ" : reference_table, notice

= Source Map and Compatibility

== Source of Truth

本書は，SPENCERが取得するA9N Repositoryを基準とする．数値，Type，Operation，ABI LayoutはHeaderから，State TransitionとError変換は実装Codeから確認した．Testと既存Markdownは補助資料として扱う．

対象Revisionには，公開Wrapper HeaderとVersioned ABI Packageが含まれない．本書は，Kernel内部Headerと実装に基づくImplementation Referenceである．Stable ABIの保証とBackward Compatibilityの期間は定義されていない．

== Chapter Source Map

#reference_table(
  (1.3fr, 2fr, 2.2fr),
  ([*Chapter*], [*Definition source*], [*Behavior source*]),
  [Capability], [`capability_component.hpp`，`capability_types.hpp`，`capability_result.hpp`], [`capability_node.cpp`と各Object実装．],
  [Kernel Call], [`kernel_call.hpp`，`types.hpp`，`fault.hpp`，`ipc_buffer.hpp`], [`kernel_call.cpp`，`_syscall.s`，`systemcall.cpp`，`process_manager.cpp`．],
  [Capability Node], [`capability_node.hpp`], [`capability_node.cpp`．],
  [Generic], [`generic.hpp`], [`generic.cpp`．],
  [PCB and Scheduler], [`process_control_block.hpp`，`process.hpp`，`scheduler.hpp`，`arch_context.hpp`], [`process_control_block.cpp`，`process_manager.cpp`，`scheduler.cpp`．],
  [IPC Port], [`ipc_port.hpp`，`ipc_buffer.hpp`], [`ipc_port.cpp`．],
  [Notification Port], [`notification_port.hpp`，`notification.hpp`], [`notification_port.cpp`．],
  [Memory], [`address_space.hpp`，`memory_type.hpp`，`page_table_capability.hpp`，`frame_capability.hpp`], [`address_space.cpp`，`x86_64/memory/memory_manager.cpp`．],
  [Interrupt and I/O], [`interrupt_region.hpp`，`interrupt_port.hpp`，`io_port_capability.hpp`], [`interrupt_region.cpp`，`interrupt_port.cpp`，`io_port_capability.cpp`，`interrupt_manager.cpp`．],
  [Boot and Init], [`boot_info.hpp`，`init.hpp`，`version.hpp`], [`main.cpp`，`boot/init.cpp`，`x86_64/arch/arch_initializer.cpp`．],
  [SMP], [`config.hpp`，`cpu.hpp`，`lock.hpp`，`process_manager.hpp`，HALの`cpu.hpp`と`interrupt.hpp`], [`process_manager.cpp`，`interrupt_manager.cpp`，`x86_64/arch/cpu.cpp`，`arch_initializer.cpp`，`interrupt.cpp`．],
  [Virtualization], [`virtual_cpu_capability.hpp`，`virtual_cpu.hpp`], [`virtual_cpu_capability.cpp`，`x86_64/virtualization/virtual_cpu.cpp`．],
  [HAL Porting], [`src/hal/include/hal/interface/*.hpp`], [`src/hal/x86_64`，Root CMake，HAL CMake．],
  [x86_64 ABI], [`src/hal/x86_64/include/hal/arch`], [`src/hal/x86_64/systemcall`，`process`，`memory`，`io`，`arch`，`virtualization`．],
)

PathはRepository Rootからの相対Pathである．同名Fileが複数Directoryに存在する場合，Definition Sourceは`src/kernel/include/kernel`または`src/hal/include/hal`配下を指す．Behavior Sourceは`src/kernel`または`src/hal/x86_64`配下を指す．

== Known Documentation Drift

Repository内の既存Markdownには，対象Revisionより前の説明が残っている．主な差分を次に示す．

#reference_table(
  (1.5fr, 1.8fr, 2.2fr),
  ([*Topic*], [*Existing document*], [*A9N implementation*]),
  [`message_info`], [`doc/kernel-call.md`はbit 9..14をTransfer Count，bit 15をKernel Flagとする．], [`ipc_port.hpp`はbit 9..12をTransfer Count，bit 13..14をSource，bit 15をReservedとする．],
  [PCB IPC Buffer], [`doc/kernel-call.md`はPCB `CONFIGURE`が`process.buffer` Pointerを設定しないとする．], [`process_control_block.cpp`はFrame Physical Addressから`process.buffer` Pointerを設定する．],
  [PCB Notification], [`doc/kernel-call.md`はNotification Port設定を`DEBUG_UNIMPLEMENTED`とする．], [`process_control_block.cpp`はNotification PortをProcessへBindする．],
  [Init I/O Port], [`doc/init.md`と`doc/kernel-init-implementation.md`はInitial I/O Portが未構成とする．], [`boot/init.cpp`はRoot Slot 9へ全RangeのI/O Portを構成する．],
  [Init Allocator], [`doc/kernel-init-implementation.md`は2 MiBのLinear Allocatorを記述する．], [`boot/init.cpp`は4 MiBのLinear Allocatorを定義する．],
  [Frame Node Size], [`doc/kernel-init-implementation.md`は16384 Slotを記述する．], [`init.hpp`は`INITIAL_FRAME_COUNT_MAX = 32768`を定義する．],
  [Node Index Test], [`test_capability_node.cpp`はRange外Indexへ`INDEX_OUT_OF_RANGE`を期待する．], [`capability_node::retrieve_slot()`は境界検査を行わない．],
)

既存Markdownの削除や自動同期は，本書のScopeに含めない．説明が競合する場合は，対象RevisionのHeaderと実装を確認する必要がある．

== Version Drift

#reference_table(
  (1.6fr, 1.4fr, 2.4fr),
  ([*Version domain*], [*Value*], [*Source*]),
  [Manual], [#raw(read("/version.txt").trim())], [`doc/a9n-manual/version.txt`．],
  [CMake Project], [`0.1.8`], [Root `CMakeLists.txt`の`project()`．],
  [Standard Kernel Build], [`0.2.1-smp+...`], [`src/kernel/CMakeLists.txt`がArchitecture，Build Type，Dateを付加する．],
  [Kernel Fallback], [`0.1.0-unknown+00000000-UNKNOWN`], [`src/kernel/include/kernel/version.hpp`．],
)

Version Domain間の一致を保証するBuild Checkは存在しない．Release時はManual Version，CMake Project Version，Kernel Versionを個別に更新し，`init_info`が返すKernel Versionを起動Testで確認する必要がある．

#notice(
  [NOTE],
  [ABIを変更するCommitでは，Operation Number，Message Register Layout，Bit Field，Error変換，Blocking State，Wakeup State，Architecture差分を本書と同時に更新する必要がある．Definition SourceとBehavior SourceをReview対象の確認に用いる．],
)
