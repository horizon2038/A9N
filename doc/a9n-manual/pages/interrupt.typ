#import "/components/reference.typ" : operations, notice, term
#import "@preview/cetz:0.5.2"

= Interrupt

== Interrupt Object Model

#term[Interrupt Region]は，IRQ番号を割り当てる権限を表すCapability Objectである．Interrupt Regionの`MAKE_PORT`は，未使用のIRQに対応するInterrupt Portを一つ作成する．#term[Interrupt Port]はIRQ番号を持ち，Notification Portを配送先としてBindする．

InitはInterrupt Region CapabilityをRoot Node Slot 8に受け取る．GenericからInterrupt RegionとInterrupt Portを作成する経路は存在しない．IRQ番号の範囲はアーキテクチャごとのABIが定める．

#figure([
  #set text(size: 7.5pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(
      stroke: 0.45pt,
      mark: (transform-shape: false, fill: black),
    )

    content((-4.8, 1.65), [$n$: IRQ番号], anchor: "west")

    for (position, body, object-name) in (
      ((-4.8, 0), [Interrupt Region], "interrupt-region"),
      ((-1.6, 0), [#align(center)[Interrupt Port \ IRQ $n$]], "interrupt-port"),
      ((1.6, 0), [Notification Port], "notification-port"),
      ((4.8, 0), [Driver Process], "driver-process"),
      ((-1.6, 1.65), [Hardware IRQ $n$], "hardware-irq"),
    ) {
      content(
        position,
        body,
        name: object-name,
        frame: "rect",
        padding: 0.5em,
        fill: luma(247),
      )
    }

    line("interrupt-region.east", "interrupt-port.west", mark: (end: ">"))
    content((-3.2, 0.5), [`MAKE_PORT`], anchor: "south")
    line("interrupt-port.east", "notification-port.west", mark: (end: ">"))
    content((0, 0.5), [`BIND`], anchor: "south")
    line("notification-port.east", "driver-process.west", mark: (end: ">"))
    content((3.2, 0.5), [Wakeup / MR], anchor: "south")
    line("hardware-irq.south", "interrupt-port.north", mark: (end: ">"))
    content((-1.42, 0.88), [Deliver], anchor: "west")
    line(
      "driver-process.south",
      (4.8, -1.45),
      (-1.6, -1.45),
      "interrupt-port.south",
      mark: (end: ">"),
    )
    content((1.6, -1.27), [`ACK` / Enable], anchor: "south")
  })
], caption: [IRQ Capabilityの作成，Binding，配送，ACK])

== Interrupt Region

#operations(
  [`MAKE_PORT = 1`], [`MR2 = IRQ number` \ `MR3 = destination node descriptor` \ `MR4 = destination index`], [未使用IRQを予約し，Destination SlotへInterrupt Portを作成する．`MR3`によって探索したSlotのTypeは`NODE`であり，`MR4`で選択する子Slotは`NONE`でなければならない．], [`INVALID_DESCRIPTOR`，`INVALID_ARGUMENT`，`ILLEGAL_OPERATION`，`FATAL`．],
)

`MR3`は，呼出しProcessのRoot Capability Nodeを起点としてDestination Node Slotを探索するためのCapability Descriptorである．`MAKE_PORT`は，Interrupt Region SlotとDestination NodeのRightsを検査しない．Interrupt Region Capabilityを保持するProcessは，受理される範囲内の任意のIRQを割り当てられる．

同じIRQへ二つ目のInterrupt Portは作成できず，`ILLEGAL_OPERATION`となる．Interrupt PortをRevokeすると，IRQの`used` FlagがClearされる．Interrupt RegionをRevokeすると，全IRQの`used` FlagとHandler Binding SlotがClearされる．

Destination Indexの範囲はNode実装で検査されない．ユーザ空間は，Destination NodeのRadixからIndexの上限を検証する必要がある．

== Interrupt Port

#operations(
  [`BIND_NOTIFICATION_PORT = 1`], [`MR2 = notification port descriptor`], [Notification Port CapabilityをIRQ Handler SlotへCopyする．Handler未Bind状態を要求する．], [`INVALID_DESCRIPTOR`，`ILLEGAL_OPERATION`，`FATAL`．],
  [`UNBIND_NOTIFICATION_PORT = 2`], [追加引数なし．], [IRQ Handler Slotを削除する．Bindされていない場合も成功を返す．], [`FATAL`．],
  [`ACK = 3`], [追加引数なし．], [IRQをHAL経由でEnableする．], [`FATAL`．],
  [`GET_IRQ_NUMBER = 4`], [追加引数なし．], [`MR2`へIRQ番号を返す．], [`FATAL`．],
)

Interrupt Portに対する操作は，Interrupt Port SlotのRightsを検査しない．`BIND_NOTIFICATION_PORT`は，Source Notification PortのTypeを検査する．IRQ配送時の`NOTIFY`には，CopyされたNotification Slotの`WRITE` Rightが必要である．

Hardware IRQが発生すると，カーネルはBind済みのNotification Portへ通知する．通知に成功した後，IRQをAcknowledgeしてDisableする．DriverはDevice Eventの処理を終えてからInterrupt Portの`ACK`を呼び，IRQを再Enableする必要がある．

#notice(
  [WARNING],
  [Driverが`ACK`を実行しない場合，対象IRQはDisable状態に残る．Device StatusをClearする前に`ACK`を実行すると，同じLevel-triggered IRQが再配送され続ける可能性がある．DriverはDevice固有のStatus確認とClearを完了してから`ACK`を実行する必要がある．],
)

Bind済みHandlerが存在しないIRQはユーザ空間へ配送されない．Bind済みHandlerがない場合，カーネルはIRQをAcknowledgeまたはDisableしないため，Interrupt Controllerの状態はHAL実装に依存する．Driverを開始する前にBindingを完了する必要がある．

== Revoke and Concurrency

Interrupt PortのRevokeはIRQ予約とNotification Bindingを解除する．

SMP Buildでは，Interrupt Port OperationとHardware IRQ HandlerがGiant Lock内でIRQ Handler TableとNotification Bindingを更新する．ただし，Interrupt Portの作成，Binding変更，Driver起動をまとめるTransactionは存在しない．Initまたは単一のResource Managerが，Driverを起動する前にこれらのOperationを順序付ける必要がある．
