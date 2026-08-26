#import "/components/reference.typ" : operations, term
#import "@preview/cetz:0.5.2"

= Notification Port

== Object Model

#term[Notification Port]は，Word幅のPending FlagとFIFOのWait Queueを持つCapability Objectである．`NOTIFY`は，Capability Slot-local IdentifierをPending FlagへBitwise ORする．`WAIT`と`POLL`はPending Flag全体を返し，内部のFlagを0に戻す．

同じBitへ複数回通知しても，Pending Bitは一つだけ立つ．異なるBitは一つのWordにまとめられる．Notification Portは，通知回数，通知順序，送信元Processを記録しない．Identifier 0ではPending Flagが変化しないため，通知に使うIdentifierには0以外のBit Maskを設定する必要がある．

== Operation Summary

#operations(
  [`NOTIFY = 1`], [追加引数なし．], [Slot-local IdentifierをPending FlagへORする．Notification Port Slotに`WRITE`を要求する．Operationは呼出し元をBlockしない．], [`PERMISSION_DENIED`，`FATAL`．],
  [`WAIT = 2`], [追加引数なし．], [Pending Flagが0でない場合は`MR2`へ返す．Pending Flagが0の場合はProcessを`BLOCKED_WAIT`へ設定する．Notification Port Slotに`READ`を要求する．], [`PERMISSION_DENIED`，`FATAL`，`ILLEGAL_OPERATION`．],
  [`POLL = 3`], [追加引数なし．], [Pending Flagを`MR2`へ返す．Pending Flagがない場合は`MR2 = 0`を返す．Notification Port Slotに`READ`を要求する．], [`PERMISSION_DENIED`，`FATAL`，`ILLEGAL_OPERATION`．],
  [`IDENTIFY = 4`], [`MR2 = identifier`], [呼出しに使用したSlotのIdentifierを変更する．Notification Port Slotに`MODIFY`を要求する．], [`PERMISSION_DENIED`，`FATAL`．],
)

== IDENTIFY

`IDENTIFY`は，`MR2`を呼出しに使用したNotification Port SlotのSlot-local Identifierとして保存する．IPC Portとは異なり，`MR2`を`message_info`として先に検査しないため，Kernelは任意のWord値を保存する．ただし，Identifier 0を設定したSlotで`NOTIFY`してもPending Flagは変化しない．

`NOTIFY`はIdentifierを引数として受け取らない．Kernelは呼出しに使用されたSlotからIdentifierを読み，Notification PortのPending FlagへBitwise ORする．`WAIT`と`POLL`では，Kernelが蓄積したFlagを`MR2`へ書く．Notification PortをProcessへBindした場合は，IPC形式のResultとしてFlagを`MR3`へ書く．したがってReceiverは，通知側がMessage Registerへ自己申告した値ではなく，KernelがCapability Slotから配送したFlagを受け取る．

Capabilityの`COPY`と`MINT`はIdentifierを複製する．同じNotification Port Objectを参照するSlotへ異なるIdentifierを設定すれば，各IdentifierのbitをEvent Source，IRQ，Device，Wakeup理由へ割り当てられる．`WAIT`または`POLL`で得たWordをBit Maskとして検査することで，一つのNotification Portから複数の発生源を識別できる．同じbitへの複数回の通知は一つにまとめられるため，IdentifierをEvent回数として使用することはできない．

Identifierを変更不能な割当てとして扱う場合は，通知側へCapabilityを配布する前にIdentifierを設定し，配布するSlotから`MODIFY`を除く必要がある．`MODIFY`を持つ通知側は，別のIdentifierへ変更してから`NOTIFY`できる．

== Delivery and Wakeup

Wait Queueが空の場合，`NOTIFY`はPending Flagを残して成功を返す．Processが待っている場合は，Queue先頭Processの`MR2`へPending Flag全体を書き，内部のFlagをClearしてProcessをReady Queueへ戻す．対象ProcessのPriorityが実行中Processより高ければ，カーネルはSchedulingを行う．

`WAIT`は，最初にPending Flagを調べる．Flagが立っている場合はBlockせずに値を返す．Flagが0の場合は，ProcessをWait Queueの末尾へ追加する．複数のProcessが待っていても，一回の`NOTIFY`が起こすProcessは一つだけである．

`POLL`はBlockしない．`MR2 = 0`は通知がないことを表すため，Identifier 0による通知と区別できない．Identifier 0を通知に用いてはならない．

#figure([
  #set text(size: 7.5pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(
      stroke: 0.45pt,
      mark: (transform-shape: false, fill: black),
    )

    content((-5.6, 0.85), [$b$: Slot-local Identifier], anchor: "west")

    content(
      (-4.8, 0),
      [#align(center)[Sender \ `NOTIFY(b)`]],
      name: "notify-sender",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )
    content(
      (-1.55, 0),
      [#align(center)[Notification Port \ `pending |= b`]],
      name: "pending-port",
      frame: "rect",
      padding: 0.5em,
      fill: luma(235),
    )
    content(
      (1.55, 0),
      [Waiterあり?],
      name: "has-waiter",
      frame: "rect",
      padding: 0.5em,
    )
    content(
      (4.75, 0),
      [#align(center)[Queue Head \ `MR2 = consume()`]],
      name: "consume-pending",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )
    content(
      (4.75, -1.55),
      [#align(center)[Process \ `READY`]],
      name: "ready-process",
      frame: "rect",
      padding: 0.5em,
      fill: luma(235),
    )
    content(
      (1.55, -1.55),
      [Pending Flagを保持],
      name: "retain-pending",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )

    line("notify-sender.east", "pending-port.west", mark: (end: ">"))
    line("pending-port.east", "has-waiter.west", mark: (end: ">"))
    line("has-waiter.east", "consume-pending.west", mark: (end: ">"))
    content((3.15, 0.18), [Yes], anchor: "south")
    line("consume-pending.south", "ready-process.north", mark: (end: ">"))
    line("has-waiter.south", "retain-pending.north", mark: (end: ">"))
    content((1.73, -0.78), [No], anchor: "west")
  })
], caption: [`NOTIFY`によるPending Flagの蓄積とWaiterのWakeup])

== Binding to a Process

PCBの`CONFIGURE` Bit 3は，Notification PortをProcessへBindする．一つのNotification PortにBindできるProcessも一つだけである．同じProcessへの再Bindは成功する．別のProcessへBindしようとすると，Kernel内部Errorが`FATAL`へ変換される．

Bind済みProcessがIPC Portで`BLOCKED_RECEIVE`になっている間に通知が届くと，ProcessをIPC Queueから外す．カーネルはIPC形式の`message_info`を`MR2`へ設定し，`source = NOTIFICATION`，`message_length = 0`，`transfer_count = 0`とする．`MR3`にはConsumeしたPending Flagを返す．

IPC Portの`CALL`待機前と`REPLY_RECEIVE`待機前も，Bind済みNotificationを検査する．Pending Notificationが存在する場合，IPC待機へ入らずNotification形式のResultを返す．

== Interrupt Delivery

Interrupt PortはNotification Port CapabilityをIRQ Handler SlotへCopyする．Hardware IRQ到着時，KernelはHandler SlotのIdentifierをPending FlagへORし，IRQをAcknowledgeしてからMaskする．DriverはNotificationを処理した後，Interrupt Portの`ACK`を実行してIRQを再Enableする．

同じIRQがMaskされるまでに発生した複数Eventは，Notification Bitへ統合され得る．Device DriverはDevice Status Registerを読んで全Pending Eventを処理する必要がある．Notificationの受信回数をEvent数として扱うと，Eventを取りこぼす可能性がある．

== Revoke and Concurrency

Notification Portの`revoke()`は空実装であり，Wait QueueとBind済みProcessを解除しない．PCB RevokeはPCB側のBindingを解除する．Notification Portを破棄する前に，WaiterをSuspendし，PCB BindingとInterrupt Bindingを解除する必要がある．

SMP BuildではGiant Lockが各Notification Operationを直列化する．Pending Flag，Wait Queue，Bind済みProcess Pointerは，一つのKernel Entry内でCore間同時更新されない．Binding変更とPCB停止，Interrupt Binding解除，Port破棄をまとめるTransactionは存在しないため，Lifecycleを変更する複数OperationはUser-level Resource Managerが順序付ける必要がある．
