#import "/components/reference.typ" : operations, notice, term
#import "@preview/cetz:0.5.2"

= Capability Node

== Object Model

#term[Capability Node]は，$2^"radix_bits"$個のCapability Slotを持つRadix Treeである．子Slotへ別のNodeを格納すると，複数階層のCapability Spaceを構成できる．Genericの`CONVERT`で作成したNodeでは，`ignore_bits = 0`となる．

Nodeに対する操作では，Capability Callの対象となったNodeを転送先とする．`MR3`のSource Descriptorによって，呼出し元ProcessのRoot Nodeを起点としてSource Slotを探索する．

== Operation Summary

#operations(
  [`COPY = 1`], [`MR2 = destination index` \ `MR3 = source descriptor`], [Source CapabilityをDestination SlotへCopyする．Target Nodeに`READ | WRITE`，Source Slotに`COPY`を要求する．Destination Slotは`NONE`でなければならない．], [`ILLEGAL_OPERATION`，`PERMISSION_DENIED`，`INVALID_ARGUMENT`，`FATAL`．],
  [`MOVE = 2`], [`MR2 = destination index` \ `MR3 = source descriptor`], [Source CapabilityをDestination SlotへMoveする．Target Nodeに`READ | WRITE`を要求する．Source Rightsは検査しない．Destination Slotは`NONE`でなければならない．], [`ILLEGAL_OPERATION`，`INVALID_ARGUMENT`，`FATAL`．],
  [`MINT = 3`], [`MR2 = destination index` \ `MR3 = source descriptor` \ `MR4 = new rights`], [Source CapabilityをCopyし，Destination Rightsを`new_rights`へ縮退する．Target Nodeに`READ | WRITE`，Source Slotに`COPY`を要求する．], [`ILLEGAL_OPERATION`，`PERMISSION_DENIED`，`INVALID_ARGUMENT`，`FATAL`．],
  [`DEMOTE = 4`], [`MR2 = target index` \ `MR4 = new rights`], [Target SlotのRightsを縮退する．Node Slotに`WRITE`，Target Slotに`READ | WRITE`を要求する．], [`ILLEGAL_OPERATION`，`PERMISSION_DENIED`，`INVALID_ARGUMENT`．],
  [`REVOKE = 5`], [`MR2 = target index`], [Target Slotの派生Slotを削除し，Object固有の`revoke()`を呼ぶ．Target Slotに`READ | WRITE`を要求する．], [`ILLEGAL_OPERATION`，`INVALID_ARGUMENT`，Object固有Error．],
  [`REMOVE = 6`], [`MR2 = target index`], [Target SlotをDependency列から外して初期化する．Node Slotに`READ`または`WRITE`，Target Slotに`READ | WRITE`を要求する．], [`PERMISSION_DENIED`，`INVALID_ARGUMENT`，`FATAL`．],
)

#figure([
  #set text(size: 7.5pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(
      stroke: 0.45pt,
      mark: (transform-shape: false, fill: black),
    )

    content(
      (0, 1.05),
      [$O$: 同一Objectへの参照，$R' subset.eq R$],
      anchor: "center",
    )
    content((-3.2, 0.45), [操作前], anchor: "center")
    content((3.2, 0.45), [操作後], anchor: "center")

    for (row, operation, before-source, after-source, after-destination) in (
      (0, [`COPY`], [#align(center)[Source \ $O, R$]], [#align(center)[Source \ $O, R$]], [#align(center)[Destination \ $O, R$]]),
      (1, [`MOVE`], [#align(center)[Source \ $O, R$]], [#align(center)[Source \ `NONE`]], [#align(center)[Destination \ $O, R$]]),
      (2, [`MINT`], [#align(center)[Source \ $O, R$]], [#align(center)[Source \ $O, R$]], [#align(center)[Destination \ $O, R'$]]),
    ) {
      let y = -0.8 - row * 1.55
      content((-5.6, y), operation, anchor: "west")
      content(
        (-4.1, y),
        before-source,
        frame: "rect",
        padding: 0.5em,
        fill: luma(247),
      )
      content(
        (-2.15, y),
        [#align(center)[Destination \ `NONE`]],
        frame: "rect",
        padding: 0.5em,
      )
      line((-0.75, y), (0.75, y), mark: (end: ">"))
      content((0, y + 0.18), operation, anchor: "south")
      content(
        (2.15, y),
        after-source,
        frame: "rect",
        padding: 0.5em,
        fill: luma(247),
      )
      content(
        (4.1, y),
        after-destination,
        frame: "rect",
        padding: 0.5em,
        fill: luma(247),
      )
    }
  })
], caption: [`COPY`，`MOVE`，`MINT`によるSlot内容の変化])

== COPY

`COPY`は，Source SlotのType，Component，Rights，Slot-local DataをDestination Slotへ複製する．カーネルはDestination SlotをSource Slotと同じDependency Depthへ挿入する．Object自体は複製しない．

`COPY`の成功後もSource Slotは有効である．Destination SlotのRightsはSource Rightsと同一である．Rights縮退を伴う複製には`MINT`を使用する．

== MOVE

`MOVE`は，Source Slotの内容とDependency LinkをDestination Slotへ移す．成功するとSource SlotのTypeは`NONE`となる．Source Slotに`COPY` Rightは必要ない．

SourceとDestinationに同じSlotを指定した場合の動作は定義されていない．ユーザ空間は，異なるSlotを指定する必要がある．

== MINT and DEMOTE

`MINT`でSource Rightsを超えるRightsは作成できない．Subsetであるかは次の式で検査する．

```text
(source_rights & new_rights) == new_rights
```

`DEMOTE`は，既存SlotのRightsを直接減らす．失われたRightsを同じSlotへ戻す操作は存在しない．同じObjectを参照する別のSlotには影響しない．

== REVOKE and REMOVE

`REMOVE`は，対象SlotにSiblingが残っている場合，Object固有のRevokeを行わない．最後のSiblingを削除する場合だけ，カーネルがObject固有の`revoke()`を呼ぶ．

`REVOKE`は，Target Slotに続くDependency列を走査し，Target Depthより深いSlotを順に削除する．削除するSlotがObjectへの最後の参照であれば，Slotの初期化前にObject固有の`revoke()`を呼ぶ．Target Slotが呼出し対象と同じNode Objectを参照する場合は，無限再帰を避けるためNode自身の`revoke()`を呼ぶ．

#notice(
  [CAUTION],
  [RevokeとRemoveは不可分ではない．Childの削除中にErrorが起きても，削除済みのSlotは元に戻らない．Objectごとの`revoke()`には空実装が残るため，操作の成功だけを見て全Resourceが解放されたと判断してはならない．],
)

== Index and Error Semantics

対象Revisionの`capability_node::retrieve_slot()`は，Indexの範囲を検査しない．Headerには`is_index_valid()`が存在するが，`retrieve_slot()`からは呼ばれない．Repository内の境界Testは実装と一致していない．

Destination Slotが使用中の場合，Low-levelのCopyまたはMoveは`kernel_error::INIT_FIRST`を返す．Node操作は`convert_kernel_to_capability_error()`によって`kernel_error::INIT_FIRST`を`FATAL`へ変換する．ユーザ空間は，Destination Slotが`NONE`であることをNodeの管理情報から保証する必要がある．

== Concurrency

NodeのSlot配列とDependency Linkに共通Lockはない．同じNodeに対する`COPY`，`MOVE`，`MINT`，`DEMOTE`，`REVOKE`，`REMOVE`は，ユーザ空間で同期する必要がある．同じDestination Indexを使う操作は，必ず直列化する．
