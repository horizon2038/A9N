#import "/components/reference.typ" : reference_table, operations, fields, notice, term
#import "@preview/bytefield:0.0.7": *
#import "@preview/cetz:0.5.2"

= Generic

== Object Model

#term[Generic]は，カーネルオブジェクトの作成に使う物理メモリ領域を表すCapabilityである．Generic SlotはBase Address，Size Bits，Device Flag，Watermarkを持つ．カーネルは，GenericのWatermarkを必要なAlignmentまで進め，後続領域からオブジェクト用メモリを割り当てる．一般的なHeap Allocatorは使用しない．

#fields(
  [`data[0]`], [Base Physical Address．],
  [`data[1] bit 0..6`], [Size Bits．Genericの範囲は$2^"size_bits"$ Byteである．],
  [`data[1] bit 7`], [Device Generic Flag．],
  [`data[2]`], [次回割当ての基準となるWatermark．],
)

#figure(
  bytefield(
    bpr: 8,
    msb: left,
    rows: (6em),
    bitheader(
      "bounds",
      0,
      7,
      text-size: 8pt,
    ),
    flag[DEVICE],
    bits(7)[SIZE BITS],
    text-size: 4pt,
  ),
  caption: [`data[1]`のBit配置],
)

Initへ渡されるGenericには`READ | WRITE`が設定され，`COPY`は設定されない．`CONVERT`は，呼出しに使ったGeneric SlotのRightsを検査しない．Generic Capabilityを保持するProcessは，対象領域からKernel Objectを作成できる．

== CONVERT

Genericが受理するOperation Numberは`CONVERT = 0`だけである．

#fields(
  [`MR0`], [Generic Capability Descriptor．],
  [`MR1`], [`0`．],
  [`MR2`], [作成する`capability_type`．],
  [`MR3`], [Type固有の`specific_bits`．],
  [`MR4`], [作成数．1以上でなければならない．],
  [`MR5`], [呼出しProcessのRoot NodeからDestination Node Slotを探索するためのCapability Descriptor．],
  [`MR6`], [Destination開始Index．],
)

カーネルは，Object Sizeに合わせてWatermarkをAlignmentし，Destination Nodeの連続するSlotへObjectを作成する．Objectを一つ作成するたびにWatermarkを更新し，作成したSlotをGenericのChildとしてDependency列へ挿入する．

割当て単位を$u = 2^"memory_size_bits"$，作成数を$c$，割当て前のWatermarkを$w$とする．割当て開始Addressは$a = "align"(w, u)$，割当て後のWatermarkは$w' = a + c u$となる．Alignment Gapを含む領域がGenericの終端を越える場合，`CONVERT`はObjectを作成しない．

#figure([
  #set text(size: 7.5pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(stroke: 0.45pt)

    rect((-5.6, 0), (-3.35, -0.9), fill: luma(235))
    content((-4.475, -0.45), [使用済み], anchor: "center")

    rect((-3.35, 0), (-2.25, -0.9), fill: luma(247))
    content((-2.8, -0.45), [Gap], anchor: "center")

    for (index, label) in ((0, [$0$]), (1, [$1$]), (2, [$dots$]), (3, [$c - 1$])) {
      let x-left = -2.25 + index * 1.35
      rect((x-left, 0), (x-left + 1.35, -0.9), fill: luma(225))
      content((x-left + 0.675, -0.45), label, anchor: "center")
    }

    rect((3.15, 0), (5.6, -0.9))
    content((4.375, -0.45), [未割当て], anchor: "center")

    for (x, label) in (
      (-5.6, [`base`]),
      (-3.35, [$w$]),
      (-2.25, [$a = "align"(w, u)$]),
      (3.15, [$w' = a + c u$]),
      (5.6, [`end`]),
    ) {
      line((x, -0.9), (x, -1.15))
      content((x, -1.35), label, anchor: "north")
    }

    line((-2.25, 0.35), (3.15, 0.35), mark: (start: "|", end: "|"))
    content((0.45, 0.55), [Object領域 $c u$ Byte], anchor: "south")
  })
], caption: [WatermarkのAlignmentと`CONVERT`による連続割当て])

#notice(
  [CAUTION],
  [`count > 1`の`CONVERT`はAtomicではない．途中でDestination Slotの設定やObjectの作成に失敗しても，作成済みのObjectと進んだWatermarkは元に戻らない．ユーザ空間は作成済みSlotを調べ，必要に応じてRevokeまたはRemoveを行う必要がある．],
)

== Type-specific Parameters

#reference_table(
  (1.4fr, 1fr, 1.5fr, 2.2fr),
  ([*Type*], [*Value*], [*specific_bits*], [*Result*]),
  [`NODE`], [`2`], [Node Radix．], [$2^"specific_bits"$ Slotを持つNodeを作成する．`try_make_capability_node()`は$1 <= "specific_bits" < W$を要求する．],
  [`GENERIC`], [`3`], [Child Size Bits．], [親と同じDevice属性を持つChild Genericを作成する．],
  [`ADDRESS_SPACE`], [`4`], [未使用．], [1 Pageを割り当て，HALでRoot Address Spaceを初期化する．],
  [`PAGE_TABLE`], [`5`], [Page Table Depth．], [1 Pageを割り当て，DepthをSlot Dataへ保存する．],
  [`FRAME`], [`6`], [Frame Size Bits．], [HALが受理するSizeのFrameを作成する．受理する値はアーキテクチャごとのABIが定める．],
  [`PROCESS_CONTROL_BLOCK`], [`7`], [未使用．], [Process Control BlockとUser Hardware Contextを初期化する．],
  [`IPC_PORT`], [`8`], [未使用．], [Identifier 0のIPC Portを作成する．],
  [`NOTIFICATION_PORT`], [`9`], [未使用．], [Identifier 0のNotification Portを作成する．],
  [`VIRTUAL_CPU`], [`13`], [未使用．], [Virtual CPU Objectを作成する．Capability CallはStubである．],
)

Node用MemoryのSize計算は，`sizeof(capability_slot) * 2^specific_bits`を含む．対象Revisionは乗算と加算のOverflowを検査しない．ユーザ空間は，計算結果がWord幅に収まり，Alignment後の領域全体がGenericの範囲内に収まるRadixだけを指定する必要がある．

#reference_table(
  (1.4fr, 1fr, 2.8fr),
  ([*Type*], [*Value*], [*Failure*]),
  [`NONE`], [`0`], [`INVALID_ARGUMENT`．],
  [`DEBUG`], [`1`], [`INVALID_ARGUMENT`．Size計算経路で拒否される．],
  [`INTERRUPT_REGION`], [`10`], [`INVALID_ARGUMENT`．Init Boot経路だけが作成する．],
  [`INTERRUPT_PORT`], [`11`], [`INVALID_ARGUMENT`．Interrupt Regionから作成する．],
  [`IO_PORT`], [`12`], [`INVALID_ARGUMENT`．Init Boot経路とI/O Portの`MINT`が作成する．],
  [`VIRTUAL_ADDRESS_SPACE`], [`14`], [`INVALID_ARGUMENT`．Type固有作成経路を持たない．],
  [`VIRTUAL_PAGE_TABLE`], [`15`], [`INVALID_ARGUMENT`．Size計算経路で拒否される．],
)

== Device Generic

Device Genericから作成できるTypeは`GENERIC`と`FRAME`だけである．別のTypeを指定すると，カーネルは`INVALID_ARGUMENT`を返す．Child GenericはDevice Flagを引き継ぐ．

Device FrameのCache属性やMemory TypeはGeneric Slotに含まれない．必要な属性をユーザ空間から指定できるかどうかは，Address SpaceのMapping ABIが定める．

== Allocation and Revoke

割当て単位はTypeごとに異なる．Nodeには，Node ObjectとSlot配列を格納できる最小の2の冪を用いる．PCB，IPC Port，Notification Port，Virtual CPUには，各Object Sizeを2の冪へ切り上げた値を用いる．Address SpaceとPage Tableには基本Page Sizeを用いる．FrameとChild Genericには`specific_bits`を用いる．

残容量は，Alignment後のWatermarkに`unit_size * count`を加えたEnd Addressで検査する．領域に収まらない場合は`ILLEGAL_OPERATION`，`count = 0`の場合は`INVALID_ARGUMENT`となる．

Genericの`revoke()`はWatermarkをBase Addressへ戻す．派生ObjectのMemoryはClearせず，残っているSlotも無効化しない．Revoke後に同じ領域を再割当てすると，古いCapabilityと新しいObjectが同じMemoryを参照し得る．ユーザ空間は，全Childと全Siblingを無効にしてからGenericをRevokeする必要がある．

== Error Summary

#reference_table(
  (1.3fr, 3.2fr),
  ([*Error*], [*Condition*]),
  [`INVALID_DESCRIPTOR`], [Destination Node Slotの探索または`NODE` Type検査に失敗した．],
  [`INVALID_ARGUMENT`], [`count = 0`，未対応Type，不正なFrame Size，Device Genericからの不許可変換，使用中Destination Slot．],
  [`ILLEGAL_OPERATION`], [Generic残容量が不足した，またはDestination Nodeが利用できない．],
  [`FATAL`], [HAL初期化，Memory Alignment，Message Registerの読出し，内部Slot操作が失敗した．],
)
