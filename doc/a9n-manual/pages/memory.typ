#import "/components/reference.typ" : reference_table, operations, fields, notice, term
#import "@preview/bytefield:0.0.7": *
#import "@preview/fletcher:0.5.8" as fletcher: diagram, node, edge

= Address Space，Page Table，and Frame

== Object Relationship

#term[Address Space]はRoot Page Tableを表すCapabilityである．#term[Page Table]は中間Page Tableを，#term[Frame]は物理メモリ範囲を表す．MappingとUnmappingはAddress Space Capability Callから実行し，Page TableとFrameは操作の引数として指定する．

Genericの`CONVERT`は，Address SpaceとPage Tableに基本Page Sizeの領域を割り当て，Frameに指定Sizeの物理メモリを割り当てる．Address Spaceの初期化，User Addressの範囲，利用できるFrame SizeはHALが定める．

Address Spaceに対する操作は，Address Space SlotのRightsを検査しない．Address Space Capabilityを保持するProcessは，対象Address SpaceのMappingを変更できる．

#figure([
  #diagram(
    node-stroke: 0.08em,
    node-fill: luma(246),
    node-inset: 0.8em,
    spacing: 2.2em,

    node((0, 0), [Generic], name: <generic>),
    node((2, 0), [Memory Capability \ Address Space / Page Table / Frame], name: <objects>),
    node((0, 2), [Page Table \ Intermediate], name: <page-table>),
    node((2, 2), [Address Space \ Root Page Table], name: <address-space>),
    node((4, 2), [Frame \ Physical Memory], name: <frame>),

    edge(<generic>, <objects>, [`CONVERT`], "-|>", label-side: center),
    edge(<address-space>, <page-table>, [`MAP` / `UNMAP`], "-|>", label-side: center),
    edge(<address-space>, <frame>, [`MAP` / `UNMAP`], "-|>", label-side: center),
  )
], caption: [Memory Capability Objectの作成とMapping操作])

== Mapping Attribute

`MAP`の`MR4`はMemory Rights Bit Maskである．

#reference_table(
  (1fr, 0.8fr, 2.7fr),
  ([*Right*], [*Value*], [*Meaning*]),
  [`READ`], [`0x1`], [読出しを許可する．Hardware Page Entryへの変換はHALが定める．],
  [`WRITE`], [`0x2`], [書込みを許可する．],
  [`EXECUTE`], [`0x4`], [命令実行を許可する．],
  [`ALL`], [`0x7`], [Read，Write，Executeを指定する．],
)

#figure(
  bytefield(
    bpr: 8,
    msb: left,
    rows: (7em),
    bitheader(
      "bounds",
      0,
      1,
      2,
      3,
      7,
      text-size: 8pt,
    ),
    bits(5)[RESERVED],
    flag[EXECUTE],
    flag[WRITE],
    flag[READ],
    text-size: 4pt,
  ),
  caption: [`MAP`のMemory Rights Bit配置],
)

Cache属性，Global属性，Memory Typeを指定するFieldは共通インターフェースに存在しない．未定義のAttribute Bitは明示的に拒否されないが，Mappingへ反映されない．ユーザ空間は`0x0..0x7`だけを使用する必要がある．

== Address Space Operations

#operations(
  [`MAP = 1`], [`MR2 = Page Table or Frame descriptor` \ `MR3 = user virtual address` \ `MR4 = attribute`], [Page TableまたはFrameをAddress SpaceへMapする．対象Entryは未使用でなければならない．], [`INVALID_DESCRIPTOR`，`INVALID_ARGUMENT`，`ILLEGAL_OPERATION`，`PERMISSION_DENIED`，`FATAL`．],
  [`UNMAP = 2`], [`MR2 = Page Table or Frame descriptor` \ `MR3 = user virtual address`], [Page TableまたはFrameに対応するEntryをClearする．], [`INVALID_DESCRIPTOR`，`INVALID_ARGUMENT`，`ILLEGAL_OPERATION`，`PERMISSION_DENIED`，`FATAL`．],
  [`GET_UNSET_DEPTH = 3`], [`MR2 = user virtual address` \ `MR3 = leaf size bits`], [指定LeafをMapするために最初に不足するPage Table Depthを`MR2`へ返す．不足がなければ0を返す．], [`INVALID_DESCRIPTOR`，`ILLEGAL_OPERATION`，`PERMISSION_DENIED`，`FATAL`．],
  [`CAN_MAP_FRAME_SIZE_BITS = 4`], [`MR2 = frame size bits`], [HALがFrame Sizeを受理する場合は成功を返す．追加Resultはない．], [`INVALID_DESCRIPTOR`，`FATAL`．],
)

== MAP

Page Table Mappingは，対象Page TableのDepthから格納先のEntry Depthを決める．受理するDepthとAddressはHALが検査する．

Frame Mappingは，Frame Size BitsからLeaf Depthを決める．必要な中間Page Tableが存在しない場合，カーネルは`INVALID_DESCRIPTOR`を返す．Size BitsとLeaf Depthの対応はアーキテクチャごとのABIが定める．

対象EntryがPresentの場合，カーネルは`ILLEGAL_OPERATION`を返す．既存のMappingを置き換える操作はない．Mappingを変更する場合は，`UNMAP`が成功した後で`MAP`を行う必要がある．

Virtual AddressとPhysical AddressのAlignmentはAddress Space Callで明示検査されない．ユーザ空間は，HALが定めるMapping Sizeに合わせて両方のAddressをAlignmentする必要がある．

== UNMAP and TLB

Page Tableの`UNMAP`は，対象EntryがNot Presentでも成功を返す．Frameの`UNMAP`は，同じ状態で`ALREADY_MAPPED`内部Errorを返し，Capability Errorは`ILLEGAL_OPERATION`となる．Page TableとFrameでは，再実行時の結果が異なる．

`UNMAP`は，対象EntryのPhysical Addressと，`MR2`のDescriptorによって探索したSlotが参照するPage TableまたはFrameのPhysical Addressを比較しない．対象CapabilityはUnmapするDepthを決めるためだけに使う．同じDepthまたはSizeを持つ別Objectを参照するCapability SlotのDescriptorでも，指定Virtual AddressのEntryをClearできる．Address Space Capabilityは，Address Space内の全Mappingを変更できる権限として扱う必要がある．

Mapping対象Address Spaceが現在Coreで実行中なら，HALはLocal TLBを無効化する．Frame操作は対象Virtual AddressのEntryだけを無効化できるが，中間Page Table操作は配下の複数Entryを無効化する必要がある．SMP Buildでは，KernelがHALからAddress Space Owner Bitmapを取得し，別CoreのBitがあればPublic HAL Interfaceを通じてTLB Shootdown IPIを送る．Remote OwnerがなければIPIを送らない．具体的なBitmapの保存場所，無効化命令，IPI，Address Space切替え時の処理はアーキテクチャごとのABIに記載する．

== GET_UNSET_DEPTH

`GET_UNSET_DEPTH`は，RootからLeafの直前までPage Tableをたどる．最初のNot Present Entryが必要とするChild Depthを返す．途中にHuge Page Entryがある場合は`ILLEGAL_OPERATION`となる．

ユーザ空間は，返されたDepthのPage TableをGenericから作成し，指定Addressへ`MAP`した後で`GET_UNSET_DEPTH`を繰り返す．Result 0は，指定Frame Sizeに必要な中間Page Tableがすべて存在することを表す．

== Page Table Capability

Page Table Capabilityの`execute()`を直接呼ぶと，すべての操作で`ILLEGAL_OPERATION`となる．Page Tableの作成時，カーネルはPage Table MemoryをZero Clearする．Page Table DepthはSlot-local Dataへ保存される．

Page Tableの`revoke()`は空実装である．Page Table CapabilityをRemoveしても，Address Space内のEntryは残る．関連するMappingを先に`UNMAP`する必要がある．

== Frame Capability

Frame Slot DataはPhysical Address，Flags，Size Bitsを保持する．Genericから作成するFrameのFlagsは0である．Address Space Mapping AttributeはFrame Slot Flagsではなく`MAP`の`MR4`から取得する．

Headerは`GET_ADDRESS = 1`と`MR2` Resultを宣言する．Frame Capabilityの`execute()`はOperation NumberをDecodeせず，常に`DEBUG_UNIMPLEMENTED`を返すため，Physical Addressを取得する公開APIとしては使用できない．

Frameの`revoke()`も空実装である．Frame CapabilityをRemoveしても，Mappingは残り，Frame MemoryもZero Clearされない．Frameを再割当てする前に，全Address SpaceからUnmapし，必要なDataをユーザ空間で消去する必要がある．

#notice(
  [WARNING],
  [Page TableやFrame CapabilityをRemoveしても，Hardware Page Table Entryは残り得る．Mappingを残したままGenericをRevokeしたりMemoryを再利用したりすると，古いAddress Spaceから別ObjectのMemoryへ到達できる．Process停止，MappingのUnmap，CapabilityのRemove，Memoryの再利用の順に破棄する必要がある．],
)

== Concurrency

SMP BuildではGiant LockがPage Table更新，Address Space Owner Bitmapの更新，Shootdown IPIの送信を直列化する．Address Space単位の追加Lockは使用しない．同じAddress Spaceを複数Coreで実行している間にMappingを変更すると，HALはLocal TLBを無効化し，KernelはBitmapに記録されたRemote CoreのTLBを無効化する．
