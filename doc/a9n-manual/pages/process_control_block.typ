#import "/components/reference.typ" : reference_table, operations, fields, notice, term
#import "@preview/bytefield:0.0.7": *
#import "@preview/cetz:0.5.2"

= Process Control Block and Scheduler

== Object Model

#term[Process Control Block]（PCB）は，ユーザ空間の実行文脈を表すCapability Objectである．PCBはHardware Context，Floating-point Context，Process Status，Priority，Quantum，CPU Affinity，Root Capability Node，Root Address Space，IPC Buffer，Notification Port，Fault Resolver Port，IPC Reply Stateを持つ．

GenericからPCBを作成すると，User Mode用のHardware ContextとFloating-point Contextが初期化される．Address Space，Root Node，IPC Buffer，Notification Port，Fault Resolver，Instruction Pointer，Stack Pointer，Thread-local Base，Priority，CPU Affinityは，`CONFIGURE`で設定する．InitのPCBだけは，Boot処理が直接構成する．

PCBに対する操作は，PCB SlotのRightsを検査しない．PCB Capabilityを保持するProcessは，対象ProcessのContext，Address Space，Scheduling Stateを変更できる．PCB Capabilityは，Process Managerだけへ配布する必要がある．

== Operation Summary

#operations(
  [`CONFIGURE = 1`], [`MR2 = configuration info` \ `MR3..MR12 = fields`], [Bit Maskで選択したPCB Fieldを順番に設定する．複数Fieldの更新はAtomicではない．], [`INVALID_DESCRIPTOR`，`ILLEGAL_OPERATION`，`INVALID_ARGUMENT`，`FATAL`．],
  [`READ_REGISTER = 2`], [`MR2 = register count`], [Hardware Contextの先頭から`register_count` Wordを`MR3..`へ返す．], [`INVALID_ARGUMENT`，`FATAL`．],
  [`WRITE_REGISTER = 3`], [`MR2 = register count` \ `MR3.. = values`], [Hardware Contextの先頭から`register_count` Wordを書き換える．], [`INVALID_ARGUMENT`，`FATAL`．],
  [`RESUME = 4`], [追加引数なし．], [Processを`READY`へ設定し，Quantumを10へ設定してReady Queueへ追加する．], [`FATAL`．],
  [`SUSPEND = 5`], [追加引数なし．], [IPC QueueまたはNotification QueueからProcessを外し，Ready Queueから削除して`BLOCKED_SUSPEND`へ設定する．], [`FATAL`．],
)

== CONFIGURE

`configuration_info`でBitが1になっているFieldだけを更新する．カーネルはBit 0からBit 9の順に処理する．途中でErrorが発生しても，更新済みのFieldは元に戻らない．

Descriptorを受け取るFieldでは，呼出しProcessのRoot Capability Nodeを起点として，Descriptorによって対象Capability Slotを探索する．探索したSlotのTypeを検査した後，CapabilityをPCB内部SlotへCopyする．

#reference_table(
  (0.5fr, 1.5fr, 0.7fr, 2.7fr),
  ([*Bit*], [*Field*], [*MR*], [*Description*]),
  [`0`], [`address_space`], [`MR3`], [Descriptorによって対象Slotを探索し，`ADDRESS_SPACE` CapabilityをPCB内部のRoot Address Space SlotへCopyする．],
  [`1`], [`root_node`], [`MR4`], [Descriptorによって対象Slotを探索し，`NODE` CapabilityをPCB内部のRoot SlotへCopyする．],
  [`2`], [`frame_ipc_buffer`], [`MR5`], [Descriptorによって対象Slotを探索し，`FRAME` CapabilityをPCB内部SlotへCopyして，Frame Physical AddressからKernel側`ipc_buffer*`を設定する．],
  [`3`], [`notification_port`], [`MR6`], [Descriptorによって対象Slotを探索し，`NOTIFICATION_PORT` CapabilityをProcessへBindして，PCB内部SlotへCopyする．既存Bindingは解除する．],
  [`4`], [`ipc_port_resolver`], [`MR7`], [Descriptorによって対象Slotを探索し，`IPC_PORT` CapabilityをFault ResolverとしてPCB内部SlotへCopyする．],
  [`5`], [`instruction_pointer`], [`MR8`], [HALがUser Addressとして受理する値をInstruction Pointerへ設定する．],
  [`6`], [`stack_pointer`], [`MR9`], [HALがUser Addressとして受理する値をStack Pointerへ設定する．],
  [`7`], [`thread_local_base`], [`MR10`], [HALがUser Addressとして受理する値をThread-local Baseへ設定する．対応するHardware ContextはアーキテクチャごとのABIが定める．],
  [`8`], [`priority`], [`MR11`], [0以上32未満のPriorityを設定する．大きい値ほど高Priorityである．],
  [`9`], [`affinity`], [`MR12`], [CPU Core Indexを保存する．対象RevisionはRange検査とScheduler適用を行わない．],
)

#figure(
  bytefield(
    bpr: 16,
    msb: left,
    rows: (14em),
    bitheader(
      "bounds",
      0,
      8,
      10,
      15,
      text-size: 8pt,
    ),
    bits(6)[RESERVED],
    flag[CPU~AFFINITY],
    flag[PRIORITY],
    flag[THREAD‑LOCAL~BASE],
    flag[STACK~POINTER],
    flag[INSTRUCTION~POINTER],
    flag[FAULT~RESOLVER~PORT],
    flag[NOTIFICATION~PORT],
    flag[IPC~BUFFER~FRAME],
    flag[ROOT~CAPABILITY~NODE],
    flag[ADDRESS~SPACE],
    text-size: 4pt,
  ),
  caption: [`configuration_info`下位16 bitの配置],
)

Bit 10以降はカーネルが解釈しない．ユーザ空間は未使用Bitを0にする必要がある．

`CONFIGURE`は，Instruction Pointer，Stack Pointer，Thread-local Baseに対応するPageがMap済みかを検査しない．HALのAddress検査も，数値の範囲だけを確認する．受理される範囲はアーキテクチャごとのABIに記載する．

#notice(
  [WARNING],
  [`frame_ipc_buffer`はFrameの物理Addressをカーネルから参照できるAddressへ変換する．Frame Size，構造体の配置，ユーザ空間のMappingは`CONFIGURE`で検査されない．IPC Bufferには専用Frameを用い，`ipc_buffer`構造体をFrame先頭へ配置する必要がある．],
)

== Register Access

`READ_REGISTER`と`WRITE_REGISTER`は，Hardware ContextをWord配列として扱う．`register_count`の上限は`HARDWARE_CONTEXT_SIZE`である．配列の長さと各Indexの意味はアーキテクチャごとのABIが定める．

#notice(
  [CAUTION],
  [対象Revisionの`READ_REGISTER`は，Hardware Contextの読出しと返却先MRへの書込みをIndex順に行う．実行中Processが自身のPCBを読む場合，返却先MRと後続の読出し対象が重なると，返却値は呼出し開始時点のSnapshotにならない．Process Managerは，停止中の別Processを読出し対象とする必要がある．自己PCBでSnapshotを維持できる範囲は，アーキテクチャごとのABIに従う必要がある．重なりが発生した後に元のContextを復元する共通Operationは存在しない．],
)

`WRITE_REGISTER`は，各WordがUser ModeのPrivilege制約を満たすかを検証しない．不正なHardware Contextは，Context Restore時のFault，Privilege境界の破壊，Kernel Crashを起こし得る．ユーザ空間のProcess Managerは，アーキテクチャごとのABIで書換えを許されたFieldだけを変更する必要がある．

== Process States

#reference_table(
  (1.5fr, 3fr),
  ([*State*], [*Meaning*]),
  [`UNUSED`], [初期または未使用状態．],
  [`READY`], [Schedulerが実行対象として扱う状態．実行中Processも`READY`を使用する．],
  [`BLOCKED_SEND`], [IPC PortでReceiverを待つSender．],
  [`BLOCKED_RECEIVE`], [IPC PortでSenderまたはBind済みNotificationを待つReceiver．],
  [`BLOCKED_REPLY`], [`CALL`のReplyを待つClient．],
  [`BLOCKED_WAIT`], [Notification Portの`WAIT`で通知を待つProcess．],
  [`BLOCKED_SUSPEND`], [PCBの`SUSPEND`または回復不能Faultで停止したProcess．],
  [`BLOCKED_FAULT`], [Fault Resolverへの配送待ちとなったProcess．],
)

#notice(
  [CAUTION],
  [`RESUME`はIdempotentではない．Ready Queueへ所属済みのProcessまたは実行中Processへ`RESUME`を再実行すると，重複追加の失敗またはQueue Linkの破損を起こし得る．User-level Process ManagerはPCBごとのStateとQueue所属を管理し，構成済みかつReady Queueへ所属していないProcessだけを`RESUME`する必要がある．],
)

`SUSPEND`は対象ProcessをWait QueueとReady Queueから外すが，対象Processが呼出し元自身である場合に即時Schedulingを実行しない．Self Suspendを実行したProcessは`BLOCKED_SUSPEND`へ設定された後もKernel Callから復帰し得る．実行中Processを停止する処理は，別のProcess Managerから`SUSPEND`するか，IPCまたはNotificationのBlocking Operationへ移す必要がある．

== Benno Scheduling

Benno Schedulingは，実行可能なProcessだけをReady Queueへ保持し，固定PriorityとPriority内Round-robinを組み合わせるScheduling手法である@ElphinstoneEtAl:2013．対象RevisionのSchedulerはBenno Schedulingを用い，Priority 0から31に対応する32本のFIFO Ready Queueを持つ．大きいPriority値ほど選択順位が高い．

Ready Queueには，`status == READY`のProcessだけを格納する．実行中Processも`READY` Stateを持つが，Ready Queueには所属しない．BlockされたProcessは，IPC PortまたはNotification PortのWait Queueへ移るか，いずれのQueueにも所属しない．`schedule()`は，空でない最高Priority QueueのHeadを取り出す．`add_process()`は，対象Priority QueueのTailへProcessを追加する．同じPriorityのProcessは，Quantum満了ごとにFIFO順で選択される．

#figure([
  #set text(size: 7.5pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(
      stroke: 0.45pt,
      mark: (transform-shape: false, fill: black),
    )
    scale(1.2)

    content((-3.65, 0.45), [(a) `schedule()`], anchor: "west")
    content((-3.45, -0.25), [`P31`], anchor: "east")
    rect((-3.2, 0), (0.6, -0.5))
    content((-1.3, -0.25), [empty], anchor: "center")

    content((-3.45, -1.05), [`P30`], anchor: "east")
    content((-2.65, -0.68), [Tail], anchor: "center")
    content((0.05, -0.68), [Head], anchor: "center")
    for (index, label) in ((0, `C`), (1, `B`), (2, `A`)) {
      let x-left = -3.2 + index * 1.1
      let fill-color = if index == 2 { luma(225) } else { white }
      rect((x-left, -0.8), (x-left + 1.1, -1.3), fill: fill-color)
      content((x-left + 0.55, -1.05), label, anchor: "center")
    }
    content(
      (2.55, -1.05),
      [選択Process `A`],
      name: "scheduled-process",
      frame: "rect",
      padding: 0.5em,
      fill: luma(240),
    )
    line((0.1, -1.05), "scheduled-process.west", mark: (end: ">"))

    content((-3.45, -1.65), [$dots$], anchor: "east")

    content((-3.45, -2.25), [`P0`], anchor: "east")
    content((-2.65, -1.88), [Tail], anchor: "center")
    content((-1.55, -1.88), [Head], anchor: "center")
    for (index, label) in ((0, `E`), (1, `D`)) {
      let x-left = -3.2 + index * 1.1
      rect((x-left, -2), (x-left + 1.1, -2.5))
      content((x-left + 0.55, -2.25), label, anchor: "center")
    }

    line((-4.6, -2.85), (4.6, -2.85), stroke: gray + 0.2pt)
    content((-4.5, -3.2), [(b) `try_direct_schedule(target)`], anchor: "west")

    content(
      (-3.1, -3.9),
      [Direct Schedule対象],
      name: "direct-target",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )
    content(
      (0.45, -3.9),
      [#align(center)[`target.priority` \ $>= "highest ready priority"$]],
      name: "direct-condition",
      frame: "rect",
      padding: 0.5em,
      fill: luma(240),
    )
    content(
      (3.55, -3.9),
      [対象を選択],
      name: "direct-selected",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )
    content(
      (0.45, -5),
      [#align(center)[`target.priority` Queue \ Tailへ追加]],
      name: "queued-target",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )
    content(
      (3.65, -5),
      [`schedule()`],
      name: "fallback-schedule",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )

    line("direct-target.east", "direct-condition.west", mark: (end: ">"))
    line("direct-condition.east", "direct-selected.west", mark: (end: ">"))
    content((2.2, -3.72), [成立], anchor: "south")
    line("direct-condition.south", "queued-target.north", mark: (end: ">"))
    content((0.67, -4.45), [不成立], anchor: "west")
    line("queued-target.east", "fallback-schedule.west", mark: (end: ">"))
  })
], caption: [Priority別Ready QueueとDirect Scheduleの選択規則])

固定Priority方式は，選択可能な高Priority Processが存在する間，低Priority Processを選択しない．対象RevisionはPriority Agingと実行時間Reservationを実装しないため，ユーザ空間のProcess ManagerはPriority設定によるStarvationを考慮する必要がある．

== Quantum and Preemption

Process Quantumの基準値`QUANTUM_MAX`は10 Tickである．`RESUME`，`YIELD`，Quantum満了による再投入は，Quantumを10へ設定する．Timer Handlerは実行中ProcessのQuantumをTickごとに1減らす．Quantumが0以下になると，ProcessをReady QueueのTailへ戻して`try_schedule_and_switch()`を実行する．

NotificationによってProcessがReadyになった場合，`schedule_if_preempted_by()`は通知先Priorityが実行中ProcessのPriorityより大きいときだけSchedulingを実行する．同Priorityの通知先はReady Queueで待機する．IPCによるDirect Scheduleは，通常のPriority Preemptionとは別の経路である．

`YIELD`は実行中Processを`READY`のままReady Queueへ戻し，次のProcessを選択する．`YIELD`はBlock Stateを作らない．同Priority Queueに別Processが存在する場合，FIFO順の次Processが選択される．

== Direct Schedule

A9NのDirect Scheduleは，L4系MicrokernelのDirect Process Switchに由来するIPC Optimizationである@ElphinstoneEtAl:2013．`try_direct_schedule(target)`は，IPCの通信相手を通常のSchedulingを介さずに選択する．対象ProcessのPriorityがReady Queue内の最高Priority以上なら，Schedulerは対象ProcessをReady Queueから外して選択する．Ready Queueに対象Processより高いPriorityのProcessが存在する場合，Schedulerは対象ProcessをReady Queueへ追加し，`schedule()`で最高Priority Processを選択する．

`try_direct_schedule_and_switch()`は，切替元Processに残るQuantumを切替先Processへ加算してからContext Switchを行う．切替先Quantumは10を超え得る．IPC Portは，待機中Receiverへ`CALL`を配送する経路とFaultをResolverへ配送する経路でDirect Scheduleを使用する．`CALL`と`REPLY_RECEIVE`を組み合わせる#term[IPC Fastpath]は「IPC Port」に記載する．

== Scheduling Limitations

CPU AffinityはPCBへ保存されるが，対象RevisionのSchedulerは参照しない．Application Processor Entryも空実装である．Ready Queueと`highest_priority`をCoreごとに分離する実装，Remote Preemption，Queue Lockは存在しない．SchedulingはSingle-coreでのみ有効である．

== Revoke and Lifetime

PCBをRevokeすると，ProcessをIPCおよびNotificationのWait Queueから外し，Suspend状態へ移す．続いて，Root Node，Root Address Space，IPC Buffer Frame，Resolver Port，Notification Portの内部Slotを削除し，Hardware ContextとFloating-point Contextを初期化し直す．

PCB Object用MemoryはGenericへ個別返却されない．PCB CapabilityをRemoveしても，同じPCBを参照するSibling Slotが残る場合，PCB Revokeは実行されない．Reply Stateを持つProcessには相手ProcessへのPointerが残るため，ユーザ空間はIPC Sessionを終了してからPCBを破棄する必要がある．
