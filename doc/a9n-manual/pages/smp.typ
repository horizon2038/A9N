#import "/components/reference.typ" : reference_table, notice, term

= Symmetric Multiprocessing

== Scope

A9N 0.2.0は，Build時に選択する#term[Symmetric Multiprocessing]（SMP）を実装する．SMPを有効にすると，x86_64 HALはBoot時に複数のCPU Coreを起動し，KernelはCoreごとのScheduler，CPU Affinity，Local APIC Timer，IDLE Processを使用する．User Processは異なるCoreで並行実行でき，IPCとNotificationはCore境界を越えてProcessをWakeupできる．

SMPは既定で無効である．A9N Repository RootでCMakeをConfigureするとき，`A9N_CONFIG_ENABLE_SMP`を`ON`にする．このOptionはRuntime設定ではなくCompile-time設定であり，変更後は対象Build DirectoryのKernelを再Buildする必要がある．

```sh
cmake -S . -B build/x86_64-smp \
  -DARCH=x86_64 \
  -DCMAKE_TOOLCHAIN_FILE=./src/hal/x86_64/toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DA9N_CONFIG_ENABLE_SMP=ON
cmake --build build/x86_64-smp
```

SMPを無効にする場合はOptionを省略するか，明示的に`-DA9N_CONFIG_ENABLE_SMP=OFF`を渡す．同じBuild Directoryで設定を切り替える場合は，CMake Cacheに記録された値を確認する必要がある．QEMUで複数Coreを公開する場合は，KernelのSMP Buildに加えてQEMUへ`-smp <core count>`を指定する．

#reference_table(
  (1.6fr, 3.4fr),
  ([項目], [v0.2.0の規約]),
  [Build Option], [`A9N_CONFIG_ENABLE_SMP`．既定値は`OFF`．],
  [最大Core数], [`CPU_COUNT_MAX = 64`．BSPを含む．],
  [Logical Core 0], [Bootstrap Processor（BSP）．InitもCore 0から開始する．],
  [Coreの追加と削除], [Boot時に全Coreを起動する．CPU HotplugとHot-unplugは実装しない．],
  [Kernel Concurrency], [SMP BuildではGiant LockによりKernel Entryを直列化する．],
  [Scheduling], [CoreごとにProcess ManagerとBenno Schedulerを持つ．自動Load Balancingは行わない．],
)

== Compile-time Path Selection

共通Kernel Codeは`kernel::SMP_ENABLED`をCompile-time定数として参照する．SMP Buildでは`spin_lock_no_owner`が#term[Giant Lock]として選択される．SP Buildでは同じ型の位置に`null_lock`が選択され，`lock()`と`unlock()`は空の`constexpr`操作になる．Core選択，Remote Scheduling，IPI送信の分岐も`if constexpr`で選択する．

この構成により，SP BuildはAP起動を行わず，`core_count()`は1を返し，Process ManagerをCore 0から直接取得する．SMPのRemote PathはSP Binaryへ残らない．IPCの同一Core PathはSMP BuildでもDirect Scheduleを維持し，通信相手のAffinityが異なる場合だけRemote Pathへ分岐する．

== Giant Lock

SMP Buildは，次のKernel Entryの先頭で共通Giant Lockを取得する．

- Kernel Call．`CAPABILITY_CALL`，`YIELD`，`DEBUG`を含む．
- Local APIC Timer Interrupt．
- External Interrupt．
- Reschedule IPI．
- User FaultおよびArchitecture Faultの配送．

LockはHandlerから戻るときに解放する．したがって，複数CoreのUser Processは並行実行するが，同時にKernel Objectを操作できるCoreは一つである．Capability Object，Ready Queue，IPC Queue，Notification Queue，Reply Stateの更新は，この直列化領域内で実行する．ObjectごとのLockはSMPの整合性境界ではない．

Reschedule IPIを送るCoreは，Remote CoreがIPI Handlerを完了するまでGiant Lockを保持して待機しない．送信側は対象ProcessをRemote Ready Queueへ追加してIPIを送信し，現在Coreの処理を続けてKernel Entryから戻る．Remote CoreはGiant Lockを取得した後にSchedulerを実行する．

#notice(
  [NOTE],
  [Giant LockはKernel実行を直列化するが，User Modeの並行実行を禁止しない．同じAddress Spaceを複数Coreで実行するときのHardware TLB整合性は別の問題であり，本章の「Memory Consistency and Limitations」に記載する．],
)

== Core Boot Sequence

x86_64 HALはACPI MADTのEnabledなLocal APIC EntryとProcessor Local x2APIC Entryを列挙する．BSPはLogical Core 0であり，APには起動時の割当て順に1以上のLogical Core番号を与える．Logical Core番号はLocal APIC IDそのものではない．HALはLogical Core番号からLocal APIC IDへの対応を保持し，Reschedule IPIの送信時に変換する．

BSPは次の順序で#term[Application Processor]（AP）を起動する．

+ AP Trampolineと一時GDTをLow Memoryへ配置する．
+ MADTからEnabled Processorを列挙し，BSP以外へINIT IPIと2回のStartup IPIを送る．
+ APがLong Modeへ移行し，CPU-local Pointer，GDT，IDT，TSS，Local APIC，Kernel Call EntryをCoreごとに初期化する．
+ BSPがInterrupt Handler，System Clock，BSP Process Manager，Shared IDLE Context，Init Processを構成するまで，APをRelease Barrierで待機させる．
+ BSPが全APをReleaseする．各APはProcess ManagerとLocal APIC Timerを初期化し，自CoreのIDLEへ移る．
+ BSPは起動対象の全CoreがIDLEへ到達したことを確認してからInitへ切り替える．

v0.2.0はBoot段階ですべてのCoreを起動することを前提とする．CoreごとのOnline FlagをRuntime Schedulingに使用せず，Affinityの有効範囲には`core_count()`が返すBoot時の起動Core数を用いる．Enabled Processorが一つだけの場合，SMP BuildでもCore 0だけで実行を継続する．

x86_64のIPI送信経路は8 bitのDestination APIC IDを用いる．MADTが255より大きいAPIC IDを要求する構成はSMP起動対象として扱えない．また，起動数は`CPU_COUNT_MAX`の64 Coreで打ち切る．

== CPU-local State and Single Kernel Stack

Kernelは最大64個の`cpu_local_variable`とKernel Stackを静的に確保する．x86_64 HALはKernel ModeのGS Baseへ現在Coreの`cpu_local_variable` Pointerを設定する．Kernel Call EntryとInterrupt EntryはGS経由で現在Process，Scratch領域，Kernel Stack Pointerを参照する．

#reference_table(
  (1.7fr, 3.3fr),
  ([CPU-local field], [役割]),
  [`kernel_stack_pointer`], [現在Coreの8 KiB Kernel Stack上端．],
  [`current_process` / `current_context`], [現在Coreで実行中のProcessとHardware Context．Unionとして同じPointer位置を共有する．],
  [`current_virtual_cpu`], [現在のVirtual CPU用Pointer．Virtualization Capability自体は未完成である．],
  [`core_number`], [0から始まるLogical Core番号．],
  [`scratch`], [Context SwitchとEntry Assemblyが使用する一時Word．],
  [`process_manager_core`], [Core固有のProcess Manager，Scheduler，Ready Queue，IDLE Process．],
  [`is_idle`], [現在CoreがIDLE Contextを使用していることを示す．],
)

A9Nは#term[Single Kernel Stack]を採用する．Kernel StackはProcessごとではなくCoreごとに一つであり，同じCore上のすべてのProcessがKernel Entry時に共有する．Context SwitchはProcess固有Kernel Stackを切り替えない．別Coreは別のKernel Stackを持つため，User Processを複数Coreで実行してもKernel Stack Memoryは重ならない．

x86_64固有のCPU-local領域は，Logical CoreごとのLocal APIC ID，GDT，IDT，TSSを保持する．TSSのRing 0 StackとInterrupt Stackは，対応するCoreのSingle Kernel Stack上端を参照する．

== Per-core Scheduler and CPU Affinity

各CoreのProcess Managerは独立したBenno Schedulerを持つ．SchedulerのReady Queueには，そのCoreへ割り当てられた実行可能な`READY` Processだけを格納する．実行中Process，Blocked Process，Suspended Process，IDLE ProcessはReady QueueのMemberではない．固定PriorityとPriority内FIFOの規則はSP Buildと同じである．

PCBの`CONFIGURE`は，`configuration_info`のBit 9と`MR12`を使用してCPU Affinityを設定する．AffinityはLogical Core番号であり，`core_count()`以上の値は`INVALID_ARGUMENT`となる．新しいPCBとInitの既定Affinityは0である．

READY Processまたは割当て先Coreで実行中のProcessに対して，別CoreへのAffinity変更はできない．Process Managerは対象をSuspendし，Remote Reschedule IPIによって対象Coreが実際にContextを切り替えた後にAffinityを変更してResumeする必要がある．v0.2.0は自動Migration，Work Stealing，Ready Queue間のLoad Balancingを実装しない．

Local APIC TimerはCoreごとにQuantumを更新する．BSPがSystem Clock Frequencyを指定し，APはRelease後に同じ周波数で各Local APIC Timerを開始する．Timer Tickは現在CoreのProcess Managerだけを操作する．

== Inter-core IPC and Notification

IPC PortとNotification Portは，通信相手のAffinityに関係なく同じKernel ObjectとWait Queueを使用する．Giant Lock内でQueue，Message，Capability Transfer，Reply Stateを更新するため，SenderとReceiverが異なるCoreに存在してもKernel Objectの更新は直列化される．

同一Core IPCでは，`CALL`から待機中ReceiverへのHandoffに従来のDirect Scheduleを使用できる．通信相手が同じCoreにあり，より高PriorityのReady Processが存在しない場合，Schedulerは通常のQueue選択を経ず通信相手へContext Switchする．`REPLY_RECEIVE`は前のClientへのReplyと次のReceiveを一つのKernel Callで処理し，Serverが再びReceiver Queueで待機すると，次の同一Core `CALL`がこのFastpathを使用できる．

通信相手が別Coreの場合，一つのCoreから別Coreへ直接Context Switchすることはできない．Kernelは次のRemote Pathを使用する．

+ Wakeup対象をAffinity先CoreのReady Queueへ追加する．
+ Logical Core番号をLocal APIC IDへ変換し，Reschedule IPIを送る．
+ 送信元ProcessがBlockした場合，送信元Coreで別のProcessまたはIDLEを選択する．
+ 対象CoreのIPI Handlerが現在Processを必要に応じてReady Queueへ戻し，そのCoreの最高Priority Processを選択する．

Notificationも同じRoutingを使用する．別CoreのWaiterまたはBind済みReceiverをWakeupすると，対象CoreのReady Queueへ追加してReschedule IPIを送る．NotificationのPending Flagは通常どおりBitwise ORで蓄積され，`NOTIFY`自体はNon-blockingである．

#notice(
  [IMPORTANT],
  [Same-core IPC FastpathとInter-core Wakeupは異なるScheduling経路である．Inter-core IPCではLocal Context Switchの代わりにRemote Ready QueueとReschedule IPIを使用するため，Same-coreのCall／Reply-receiveと同じLatencyを仮定してはならない．],
)

== IDLE

BSPはShared IDLE Hardware Contextと一つのKernel Address Spaceを一度だけ初期化する．各Process Managerは，そのContextとAddress Spaceを使用するCore-local IDLE Processを持ち，IDLEのAffinityを自Coreへ設定する．IDLEはReady Queueへ入らず，実行可能なUser Processがない場合のScheduler Fallbackとしてだけ選択する．

x86_64 IDLEは，現在CoreのSingle Kernel Stack上端へ`RSP`を戻し，`sti`と`hlt`を隣接して実行する．Interruptが到着すると同じCoreのInterrupt Entryへ入り，TimerまたはReschedule IPIがUser Processを選択できる．Busy LoopによるPollingは行わない．

== Memory Consistency and Limitations

Giant LockはPage Table Capability，Address Space Capability，Address Space Owner Bitmapの更新を直列化する．Memory HAL Interfaceは`read_address_space_owners()`と`write_address_space_owners()`によりBitmap全体の読み書きだけを提供する．x86_64 HAL内部のContext SwitchとAddress Space作成はArchitecture固有Helperを直接使用する．Remote Coreの選択とShootdown IPIの送信はKernelのAddress Space Capabilityが行う．x86_64ではLower-half User MappingとKernel Direct Mapのどちらにも使わないPML4[511]を予約するため，Address Spaceごとの追加ObjectやGlobal Arrayを必要としない．

同じAddress Spaceを複数CoreのProcessで使用している間にMappingを変更すると，HALは`CR3`と対象PML4を直接比較してLocal TLBを無効化し，KernelはOwner Bitmapに記録されたRemote CoreへTLB Shootdown IPIを送る．Remote Handlerは現在の`CR3`を再LoadしてTLBをFlushする．Remote OwnerがないAddress SpaceにはIPIを送らない．

v0.2.0には，次の機能が含まれない．

- CPU Hotplug，Hot-unplug，RuntimeのOnline／Offline管理．
- Processの自動MigrationとLoad Balancing．
- x2APIC Destination IDを用いるIPI送信．
- IPIによるCore Haltの完成した制御経路．

SMPの機能検証では，Boot成功だけでなく，異なるAffinityを設定したProcess間の`CALL`／`REPLY_RECEIVE`，Notification Wakeup，Remote Suspend，CoreごとのTimer Preemption，IDLEからのIPI Wakeupを確認する必要がある．TLB Shootdown Testは，同じAddress Spaceを別Coreで実行し，Remote Coreが対象Virtual AddressのTranslationをCacheした後で別FrameへMappingを変更し，新しいFrameを参照することを確認する．
