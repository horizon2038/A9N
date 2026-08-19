#import "/components/reference.typ" : operations, notice, term
#import "@preview/cetz:0.5.2"

= I/O Port

== Object Model

#term[I/O Port Capability]は，HALが提供するI/O SpaceへのRead／Write権限を表すKernel Objectである．I/O Addressの意味と実際のAccess方法はHALが定める．Port-mapped I/Oを持つArchitectureだけに限定されたCapabilityではない．

Capability Slotは，許可するI/O Addressの閉区間を`data[0]`の`min`と`data[1]`の`max`に持つ．InitはRoot Node Slot 9に`min = 0`，`max = UINTMAX_MAX`のCapabilityを受け取る．Genericから作成する経路はない．Driver用の範囲は`MINT`で作成する．

== Operation Summary

#operations(
  [`READ = 1`], [`MR2 = source address` \ `MR3 = byte width`], [I/O Addressから値を読み，`MR2`へ返す．Slotに`READ`を要求する．受理するAddressとWidthはHALが定める．], [`PERMISSION_DENIED`，`FATAL`．],
  [`WRITE = 2`], [`MR2 = destination address` \ `MR3 = byte width` \ `MR4 = data`], [I/O Addressへ値を書き込む．Slotに`WRITE`を要求し，AddressがSlotの範囲内であることを要求する．], [`PERMISSION_DENIED`，`FATAL`．],
  [`MINT = 3`], [`MR2 = new min` \ `MR3 = new max` \ `MR4 = destination node descriptor` \ `MR5 = destination index`], [呼出しに使用したCapabilityの範囲に含まれるI/O Port CapabilityをDestination Slotへ作る．], [`INVALID_ARGUMENT`，`PERMISSION_DENIED`，`FATAL`．],
)

== Address Range

`WRITE`は，指定したAddressが`min <= address <= max`を満たす場合だけHALを呼ぶ．`new_min > new_max`は`INVALID_ARGUMENT`となる．`MINT`で親の範囲外を指定すると`PERMISSION_DENIED`となる．`MR4`のCapability Descriptorによって，呼出しProcessのRoot Capability NodeからDestination Node Slotを探索する．Destination Node SlotのTypeは`NODE`であり，`MR5`で選択する子Slotは`NONE`でなければならない．

`MINT`は，Source Slotの`COPY`と`MODIFY`，Destination NodeのRightsを検査しない．作成したSlotには`ALL`を設定する．I/O Port Capabilityを保持するProcessは，自身が持つ範囲内でRights `ALL`の子Capabilityを作成できる．

#figure([
  #set text(size: 7.5pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(stroke: 0.45pt)

    line((-5.4, 0.45), (5.4, 0.45), mark: (start: "|", end: "|"))
    content((0, 0.65), [Parent Capability Range], anchor: "south")

    rect((-5.4, 0), (5.4, -1), fill: luma(247))
    rect((-1.8, 0), (2.2, -1), fill: luma(225))
    content((0.2, -0.5), [Minted Range], anchor: "center")

    for (x, label) in (
      (-5.4, [`min`]),
      (-1.8, [`new_min`]),
      (2.2, [`new_max`]),
      (5.4, [`max`]),
    ) {
      line((x, -1), (x, -1.25))
      content((x, -1.45), label, anchor: "north")
    }

    content(
      (0, -2.25),
      [`min <= new_min <= new_max <= max`],
      anchor: "center",
    )
  })
], caption: [I/O Port Capabilityの閉区間と`MINT`による派生Range])

#notice(
  [WARNING],
  [`READ`は対象RevisionでAddress Rangeを検査しない．`READ` Rightを持つI/O Port Capabilityは，Slot-local Dataの範囲外もHALへ渡せる．I/O Accessを委譲する場合は，HALまたは上位ServiceでRead範囲を検証する必要がある．],
)

== HAL Boundary and Lifetime

`READ`と`WRITE`は，Address，Byte Width，Dataを`read_io_port()`または`write_io_port()`へ渡す．受理するAddress空間，Width，Access命令，Dataの切捨て規則はArchitecture ABIが定める．HAL Errorは`FATAL`へ変換される．

I/O Portの`revoke()`は処理を行わない．派生Slotの削除はCapability Dependencyの処理に従う．Hardware側の状態を初期化する処理は呼ばれない．
