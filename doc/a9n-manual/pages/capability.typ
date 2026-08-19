#import "/components/reference.typ" : reference_table, fields, notice, term
#import "@preview/bytefield:0.0.7": *
#import "@preview/cetz:0.5.2"

= Capability

== Purpose and Authority

#term[Capability]は，Kernel Objectに対する操作権限を表す，カーネル管理の参照である．#term[Capability Slot]は，Object実体を指す`component`，`capability_type`，Rights，3 WordのSlot-local Data，Dependency情報を持つ．ユーザ空間はSlotのAddressを直接扱わず，#term[Capability Descriptor]でSlotを指定する．

Capability Callを実行するには，対象Slotへ到達でき，SlotのTypeが操作と一致し，必要なRightsが設定されていなければならない．Object固有の操作では，追加のDescriptorや引数も検査する．

Capabilityは，ObjectのMemory Addressをユーザ空間へ公開しない．`FRAME::GET_ADDRESS`は宣言されているが，対象Revisionの`FRAME` Capability Callは`DEBUG_UNIMPLEMENTED`を返す．

== Object-Capability Model and ACL

Object-Capability Modelでは，Objectを指定する参照とAuthorityを，カーネルが管理するCapabilityとして一体化する@DennisEtAl:1966．A9Nでは，ProcessのRoot Capability NodeからCapability Descriptorによって到達可能なCapability Slotが，Object Operationを呼び出すAuthorityの根拠となる．Rightsを要求するOperationでは，Capability SlotのRightsも検査する．Capability DescriptorはSlotの探索位置を表す値であり，Descriptor値だけを知っていてもAuthorityは得られない．

A9NのObject-Capability Modelは，KeyKOS，EROS，seL4の設計事例から影響を受けている．KeyKOSはObjectへのKeyをAuthorityとして保持し，Keyを介したMessageでObject Operationを呼び出す@Hardy:1985．EROSは，Process，Node，PageをCapabilityによって参照するObjectとして構成し，Commodity Processor上でCapability-based Architectureを実装した@ShapiroEtAl:1999．seL4は，Objectへの参照とRightsを持つCapabilityをCNodeへ格納し，ThreadのRoot CNodeから到達できるCSpaceをAuthorityの範囲とする@seL4ReferenceManual:2026．A9NのCapability Slot，Capability Node，Root Capability Node，Capability Descriptorによる探索は，同じObject-Capability Modelに基づくが，Object TypeとOperation，Descriptor Encoding，Rightsの意味はA9N固有である．

#term[Access Control List]（ACL）は，Objectごとに，呼出し主体のIdentityと許可するOperationを対応付けたEntryを保持し，要求されたOperationをEntryと照合するAccess Control Modelである@SaltzerEtAl:1973．ACLでは，Object側のEntryを更新してAuthorityを変更する．Object-Capability Modelでは，Capabilityの`COPY`，`MOVE`，`MINT`によってAuthorityを委譲する．

#block(breakable: false)[
  主要な違いは，ACLがObject側のListと呼出し主体のIdentityを判定根拠とするのに対し，Object-Capability Modelが呼出し主体から到達可能なCapabilityを判定根拠とする点にある．A9NのKernel Objectは，Process IdentityとRightsを対応付ける共通ACLを持たない．

  #figure([
    #set text(size: 7.5pt)
    #cetz.canvas({
      import cetz.draw: *

      set-style(
        stroke: 0.45pt,
        mark: (transform-shape: false, fill: black),
      )
      scale(1.05)

      content((-5.1, 0.6), [(a) Access Control List], anchor: "west")
      content(
        (-4.35, -0.4),
        [Process A],
        name: "acl-process-a",
        frame: "rect",
        padding: 0.5em,
        fill: luma(247),
      )
      content(
        (-4.35, -1.3),
        [Process B],
        name: "acl-process-b",
        frame: "rect",
        padding: 0.5em,
        fill: luma(247),
      )
      content(
        (-1.55, -0.85),
        [Object X],
        name: "acl-object",
        frame: "rect",
        padding: 0.5em,
        fill: luma(240),
      )
      content(
        (2.6, -0.85),
        [#align(center)[ACL \ Process A: `READ | WRITE` \ Process B: `READ`]],
        name: "acl-list",
        frame: "rect",
        padding: 0.5em,
        fill: luma(247),
      )

      line("acl-process-a.east", "acl-object.west", mark: (end: ">"))
      line("acl-process-b.east", "acl-object.west", mark: (end: ">"))
      content((-3.0, 0.15), [Identity + Operation], anchor: "center")
      line("acl-object.east", "acl-list.west", mark: (end: ">"))
      content((0, -0.5), [ACLを照合], anchor: "center")

      line((-5.1, -2.05), (5.1, -2.05), stroke: gray + 0.2pt)
      content((-5.1, -2.4), [(b) Object-Capability Model], anchor: "west")
      content(
        (-4.35, -3.65),
        [Process A],
        name: "ocap-process-a",
        frame: "rect",
        padding: 0.5em,
        fill: luma(247),
      )
      content(
        (-4.35, -5.45),
        [Process B],
        name: "ocap-process-b",
        frame: "rect",
        padding: 0.5em,
        fill: luma(247),
      )
      content(
        (-1.35, -3.65),
        [#align(center)[Capability A \ Object X \ `READ | WRITE`]],
        name: "ocap-cap-a",
        frame: "rect",
        padding: 0.5em,
        fill: luma(247),
      )
      content(
        (-1.35, -5.45),
        [#align(center)[Capability B \ Object X \ `READ`]],
        name: "ocap-cap-b",
        frame: "rect",
        padding: 0.5em,
        fill: luma(247),
      )
      content(
        (3.9, -4.55),
        [Object X],
        name: "ocap-object",
        frame: "rect",
        padding: 0.5em,
        fill: luma(240),
      )

      line("ocap-process-a.east", "ocap-cap-a.west", mark: (end: ">"))
      line("ocap-process-b.east", "ocap-cap-b.west", mark: (end: ">"))
      content((-2.85, -2.85), [Descriptorで探索], anchor: "center")
      content((-2.85, -4.65), [Descriptorで探索], anchor: "center")
      line("ocap-cap-a.east", "ocap-object.west", mark: (end: ">"))
      line("ocap-cap-b.east", "ocap-object.west", mark: (end: ">"))
      content((1.5, -3.65), [Capability Call], anchor: "center")
      content((1.5, -5.45), [Capability Call], anchor: "center")
      line("ocap-cap-a.south", "ocap-cap-b.north", mark: (end: ">"))
      content((-0.35, -4.55), [`COPY` / `MINT`], anchor: "west")
    })
  ], caption: [ACLとObject-Capability ModelにおけるAuthorityの所在])
]

== Capability Rights

#reference_table(
  (1fr, 0.8fr, 2.8fr),
  ([*Right*], [*Value*], [*Meaning*]),
  [`NONE`], [`0x00`], [操作権限を持たない．],
  [`READ`], [`0x01`], [Objectからの受信，読出し，待機に使用する．具体的な検査はObject Operationが定義する．],
  [`WRITE`], [`0x02`], [Objectへの送信，書込み，変更に使用する．具体的な検査はObject Operationが定義する．],
  [`COPY`], [`0x04`], [Nodeの`COPY`と`MINT`でSource Slotを複製できる．],
  [`MODIFY`], [`0x08`], [IPC PortとNotification PortのSlot-local Identifierを変更できる．],
  [`ALL`], [`0x0f`], [`READ | WRITE | COPY | MODIFY`を表す．],
)

Rightsの意味は操作ごとに異なる．Rights Bitだけを見て，全Objectに共通する読出し・書込みの規則があると解釈してはならない．Nodeの`MOVE`はSource SlotのRightsを検査せず，Destination Node Slotに`READ | WRITE`を要求する．

`MINT`と`DEMOTE`に渡す`new_rights`は，Source RightsのSubsetでなければならない．対象Revisionは未定義Bitを拒否せず，Subsetであるかだけを検査する．ユーザ空間は`0x0f`以外のBitを設定してはならない．

== Slot-local Identifier

#term[Slot-local Identifier]は，IPC PortまたはNotification Portを参照するCapability Slotの`data[0]`に保存する1 Word値である．IdentifierはPort ObjectそのもののIDでも，ProcessやThreadのGlobal IDでもない．同じPort Objectを参照する複数のCapability Slotは，それぞれ異なるIdentifierを保持できる．

`IDENTIFY`は，`MR2`で指定したIdentifierを呼出しに使用したSlotだけへ保存する．この操作には`MODIFY` Rightが必要である．`COPY`と`MINT`はSlot-local Dataを複製するため，作成されたSlotはSource Slotと同じIdentifierを持つ．その後に一方のSlotを`IDENTIFY`しても，他方のSlotは変化しない．Genericから作成したIPC PortとNotification Portの初期Identifierは0である．

Identifierの配送はKernelが行う．IPC Portでは，Kernelが送信に使用されたSlotのIdentifierを取り出し，Messageとは別にReceiverの`MR3`へ書く．Notification Portでは，Kernelが`NOTIFY`に使用されたSlotのIdentifierをPending FlagへBitwise ORし，`WAIT`または`POLL`の`MR2`へ返す．Notification PortをProcessへBindした場合は，Pending FlagをIPC形式のResultに含め，`MR3`へ返す．呼出し側がPayloadへ自己申告した値をReceiverが識別情報として信用する方式ではない．

#figure([
  #set text(size: 7.5pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(
      stroke: 0.45pt,
      mark: (transform-shape: false, fill: black),
    )

    content((-4.7, 0.35), [#align(right)[IPC Port]], anchor: "east")
    content(
      (-3.35, 0.35),
      [#align(center)[Capability Slot \ `data[0] = x`]],
      name: "ipc-identifier-slot",
      frame: "rect",
      padding: 0.75em,
      fill: luma(247),
    )
    content(
      (0.05, 0.35),
      [#align(center)[Kernel \ Message配送]],
      name: "ipc-identifier-kernel",
      frame: "rect",
      padding: 0.75em,
      fill: luma(235),
    )
    content(
      (3.65, 0.35),
      [#align(center)[Receiver \ `MR3 = x`]],
      name: "ipc-identifier-result",
      frame: "rect",
      padding: 0.75em,
      fill: luma(247),
    )

    line("ipc-identifier-slot.east", "ipc-identifier-kernel.west", mark: (end: ">"))
    line("ipc-identifier-kernel.east", "ipc-identifier-result.west", mark: (end: ">"))

    line((-5.55, -0.85), (5.25, -0.85), stroke: gray + 0.2pt)

    content((-4.7, -2.05), [#align(right)[Notification \ Port]], anchor: "east")
    content(
      (-3.35, -2.05),
      [#align(center)[Capability Slot \ `data[0] = b`]],
      name: "notification-identifier-slot",
      frame: "rect",
      padding: 0.75em,
      fill: luma(247),
    )
    content(
      (0.05, -2.05),
      [#align(center)[Kernel \ `pending |= b`]],
      name: "notification-identifier-kernel",
      frame: "rect",
      padding: 0.75em,
      fill: luma(235),
    )
    content(
      (3.65, -2.05),
      [#align(center)[Delivery \ `MR2/MR3 = flags`]],
      name: "notification-identifier-result",
      frame: "rect",
      padding: 0.75em,
      fill: luma(247),
    )

    line(
      "notification-identifier-slot.east",
      "notification-identifier-kernel.west",
      mark: (end: ">"),
    )
    line(
      "notification-identifier-kernel.east",
      "notification-identifier-result.west",
      mark: (end: ">"),
    )
  })
], caption: [Slot-local IdentifierのKernelによる配送])

Kernelが保証するのは，呼出しに使用されたCapability Slotに保存された値を，Object固有の規則で配送することである．Identifierの一意性や意味はKernelが決めない．User-level Systemは，IPC Client，Session，Capabilityの役割，Requestの配送先を識別するLabelとして利用できる．Notificationでは，各bitをDevice，IRQ，Event種別，Wakeup理由へ割り当て，複数のEvent Sourceを一つのPortへ多重化できる．

Identifierを信頼できる識別情報として使う場合，Authorityを配布する側が値を設定し，`MINT`または`DEMOTE`によって受領側Slotから`MODIFY`を除く必要がある．`MODIFY`を保持する主体はIdentifierを変更できるため，そのSlotから届くIdentifierを変更不可能な識別情報として扱うことはできない．

== Capability Descriptor

Capability Descriptorは，Root Nodeを起点としてCapability Slotを探索するための1 Word Addressである．上位8 bitには#term[Encoded Depth]を置き，残りのbitを#term[Descriptor Payload]として用いる．Word幅を$W$，Encoded Depthを$E$，Descriptor Payloadを$P$，復元後の#term[Depth]を$D$とすると，Descriptor $d$は次式で表せる．

$
  d &= E times 2^(W - 8) + P, \
  D &= E + 8.
$

Depthを直接格納する形式ではない．ユーザ空間は，目的の探索境界$D$から$E = D - 8$を求め，上位8 bitへ格納する．$E$の範囲は$0 <= E <= 255$，$P$の範囲は$0 <= P < 2^(W - 8)$である．一般化した符号化処理は次の通りである．

```text
encoded_depth = depth - 8
raw_descriptor = (encoded_depth << (WORD_BITS - 8)) | descriptor_payload
```

`extract_depth()`は上位8 bitを取り出した後で8を加える．上位8 bitが0の場合もDepthは8となる．`traverse_slot()`はRoot NodeのSlotを選択した後でDepthを比較するため，Depth 8ではCapabilityを指定できない．Capability Callで指定できる最小Depthは，$8 + i_0 + r_0$である．

=== Node Indexの算出

探索は`traverse_slot(d, D, 8)`から始まる．第$j$段のNodeへ入る前の`descriptor_used_bits`を$u_j$，Nodeの`ignore_bits`を$i_j$，`radix_bits`を$r_j$とする．初期値は$u_0 = 8$である．

#term[Ignore Bits]は読み飛ばすbit数であり，#term[Radix Bits]はNode Indexとして読むbit数である．`calculate_capability_index()`に合わせ，Maskを$M_j$，右Shift量を$S_j$，#term[Node Index]を$I_j$とすると，算出式は次の通りである．

$
  M_j &= 2^(r_j) - 1, \
  S_j &= W - (u_j + i_j + r_j), \
  I_j &= floor(d / 2^(S_j)) " mod " 2^(r_j), \
  u_(j + 1) &= u_j + i_j + r_j.
$

剰余演算は，右Shift後の値へ$M_j$をBitwise ANDする処理と等価である．実装との対応は次の通りである．

```text
mask_bits  = (1 << radix_bits) - 1
shift_bits = WORD_BITS - (ignore_bits + radix_bits + descriptor_used_bits)
index      = (descriptor >> shift_bits) & mask_bits
new_used_bits = descriptor_used_bits + ignore_bits + radix_bits
```

Nodeは`slot[index]`を選択した後，$u_(j+1)$とDepthを比較する．両者が等しければ選択したSlotを探索結果として返す．$u_(j+1) < D$の場合は，Slotが参照するCapability Componentへ探索を引き継ぐ．探索継続時に空Slotへ到達した場合は`EMPTY`，Depthに達する前にNode以外のCapabilityへ到達した場合は`TERMINAL`となる．Capability Callは，両Errorを`INVALID_DESCRIPTOR`へ変換する．

#block(breakable: false)[
  Depthは，探索対象までに消費するすべてのIgnore BitsとRadix Bitsを含まなければならない．探索対象が第$k$段のNodeに属する場合，必要なDepthは次式となる．

  $
    D = 8 + sum_(j = 0)^k (i_j + r_j).
  $
]

対象RevisionのInit Root NodeとGenericから作成するNodeは，いずれも`ignore_bits = 0`である．`ignore_bits = 0`のCapability Spaceでは，Descriptor Payloadを上位側から各Nodeの`radix_bits`ずつ分割する．

=== Addressingの例

具体例として，Word幅$W = 64$のCapability Spaceを考える．Root Node，$"Node"_1$，$"Node"_2$の`ignore_bits`はすべて0とし，次の接続を用いる．

#figure([
  #cetz.canvas({
    import cetz.draw: *
    group(name: "addressing", {
      set-style(
        stroke: 0.4pt,
        grid: (
          stroke: gray + 0.2pt,
          step: 1,
        ),
        mark: (
          transform-shape: false,
          fill: black,
        ),
      )

      scale(2)

      let box_width = 6
      let box_width_half = box_width / 2
      let box_height = 0.5
      let box_height_half = box_height / 2
      let padding = 0.2

      let calculate_pos_x(x) = {
        return x - box_width_half
      }

      rect((-box_width_half, 0), (box_width_half, -box_height), name: "address_box")

      for (i, descriptor_index, radix) in (
        (0, "0x02", 0x08),
        (1, "0x03", 0x0a),
        (2, "0x04", 0x06),
      ) {
        let x = calculate_pos_x(i * 2)
        line((x, 0), (x, -box_height))
        content(
          (x + 1, -box_height_half),
          [#descriptor_index (Radix: #str(radix))],
          anchor: "center",
        )

        let x_l = (calculate_pos_x(i * 2) + 1) - (padding + 0.25)
        let x_r = (calculate_pos_x(i * 2) + 1) + (padding + 0.25)
        let y_u = -(box_height + padding)
        let y_d = -(box_height + 2)

        rect((x_l, y_u), (x_r, y_d), anchor: "center")

        for j in range(5) {
          let step = (y_u - y_d) / 5
          line((x_l, y_u - (step * j)), (x_r, y_u - (step * j)))

          if (j == 3) {
            rect(
              (x_l, y_u - (step * j)),
              (x_r, y_u - (step * (j + 1))),
              fill: luma(240),
            )
            let x_m = (x_l + x_r) / 2
            let y_m = ((y_u - (step * j)) + (y_u - (step * j + 0.25 + 0.03))) / 2
            content((x_m, y_m - 0.03), str(descriptor_index), anchor: "center")

            let next_x_l = (calculate_pos_x((i + 1) * 2) + 1) - (padding + 0.25)

            line(
              (x + 0.25, -0.5),
              (x + 0.25, y_m),
              (x_l, y_m),
              mark: (end: ">"),
            )

            if (i >= 2) {
              continue
            }

            line(
              (x_r, y_m),
              (x_r + 0.3, y_m),
              (x_r + 0.3, y_m + 1),
              (next_x_l, y_m + 1),
              mark: (end: ">"),
            )
          }
        }

        let x_m = (x_l + x_r) / 2
        content(
          (x_m, y_d - 0.2),
          [$"Node"_#i$ (Size = $2^#radix$)],
          anchor: "center",
        )
      }
    })
  })
], caption: [Capability構成の例])

Root Nodeは$2^8 = 256$ Slot，$"Node"_1$は$2^10 = 1024$ Slot，$"Node"_2$は$2^6 = 64$ Slotを持つ．Target Capabilityまでに消費するbit数は次の通りである．

$
  D_"target" = 8 + 8 + 10 + 6 = 32.
$

先頭の8はEncoded Depth Field自体の幅である．Descriptorへ格納する値は$D_"target"$ではなく，$E = 32 - 8 = 24 = "0x18"$となる．Descriptor Payloadは，各Node Indexを所定のbit位置へ配置して作る．

$
  P &= 2 times 2^48 + 3 times 2^38 + 4 times 2^32 \
    &= "0x0002_00c4_0000_0000", \
  d &= 24 times 2^56 + P \
    &= "0x1802_00c4_0000_0000".
$

#figure(
  bytefield(
    bpr: 64,
    msb: left,
    rows: (5em),
    bitheader(
      "offsets",
      0,
      32,
      38,
      48,
      56,
      text-size: 8pt,
    ),
    bits(8)[0x18 DEPTH],
    bits(8)[0x02 ROOT],
    bits(10)[0x003 NODE1],
    bits(6)[0x04 N2],
    bits(32)[UNUSED],
    text-size: 4pt,
  ),
  caption: [`0x1802_00c4_0000_0000`のBit配置],
)

#block(breakable: false)[
  各段の計算結果を次に示す．

  #reference_table(
    (0.55fr, 1.1fr, 0.8fr, 0.8fr, 0.8fr, 1.25fr),
    ([段階], [Node], [$u_j$], [$s_j$], [Index], [判定]),
    [`0`], [Root Node], [`8`], [`48`], [`0x02`], [$u_1 = 16 < D$],
    [`1`], [`Node 1`], [`16`], [`38`], [`0x003`], [$u_2 = 26 < D$],
    [`2`], [`Node 2`], [`26`], [`32`], [`0x04`], [$u_3 = 32 = D$],
  )
]

Root Nodeはbit 55..48から`0x02`を読み，$"Node"_1$へ進む．$"Node"_1$はbit 47..38から`0x003`を読み，$"Node"_2$へ進む．$"Node"_2$はbit 37..32から`0x04`を読む．更新後のUsed BitsがDepth 32と一致するため，`slot[0x04]`に格納されたTarget Capabilityが探索結果となる．下位32 bitは探索に使用しないため，ユーザ空間は0で埋める必要がある．

途中のNode自身も指定できる．Root Nodeの`slot[0x02]`に格納された$"Node"_1$を指定する場合，探索はRoot Nodeだけで終わる．必要なDepthは$D = 8 + 8 = 16$，Encoded Depthは$E = 8$であり，Descriptorは次の値となる．

```text
raw_descriptor = 0x0802_0000_0000_0000
```

Depthを32のままにすると探索は$"Node"_1$で止まらず，後続のIndexを読み続ける．DepthはCapabilityのTypeではなく，探索を終了するSlot境界を指定する値である．

#notice(
  [WARNING],
  [対象Revisionは，復元したDepthがWord幅以下であることを検査しない．Word幅を超えるDepthやNode境界と一致しないDepthは，Shift量のUnderflowを招く．ユーザ空間は$8 < D <= W$を満たし，かつ$D = 8 + sum(i_j + r_j)$となるDescriptorだけを生成する必要がある．`retrieve_slot(index)`を直接用いるNode操作もIndex上限を検査しないため，Operation引数のIndexは$0 <= "index" < 2^"radix_bits"$へ制限する必要がある．],
)

== Copy，Move，Mint，Remove，Revoke

`COPY`は，Source Slotと同じObject，Rights，Slot-local Dataを持つSibling Slotを作る．`MOVE`は，Objectへの参照，Rights，Slot-local Data，Dependency上の位置をDestination Slotへ移し，Source Slotを`NONE`に戻す．`MINT`は`COPY`を行った後，Destination Rightsを`new_rights`へ制限する．

`REMOVE`は対象Slotを空に戻す．同じObjectを参照するSiblingが残っている場合は，対象SlotだけをDependency列から外す．最後のSiblingを削除する場合は，Object固有の`revoke()`を呼んだ後でSlotを初期化する．

`REVOKE`が行う処理はObjectごとに異なる．GenericはWatermarkをBase Addressへ戻す．Process Control BlockはProcessを停止して内部参照を外す．Interrupt PortはIRQ Bindingを解除する．Page Table，Frame，IPC Port，Notification Port，I/O Portには，限定的な処理や空の`revoke()`が残る．対象Revisionは，参照整合性を維持したままObject用Memoryを再利用する手順を定めていない．

== Common Result

Capability Callは，共通Resultを`MR0`と`MR1`へ返す．操作固有のResultは`MR2`以降へ返す．

#fields(
  [`MR0 = 1`], [操作が成功した．`MR1`の値は定義されない．],
  [`MR0 = 0`], [操作が失敗した．`MR1`に`capability_error`を返す．],
)

#reference_table(
  (0.7fr, 1.3fr, 3fr),
  ([*Value*], [*Error*], [*Meaning*]),
  [`0`], [`ILLEGAL_OPERATION`], [操作がType，State，Rights，または残容量に適合しない．],
  [`1`], [`PERMISSION_DENIED`], [必要なRightsがない，または指定した権限がSourceの権限を超える．],
  [`2`], [`INVALID_DESCRIPTOR`], [Capability DescriptorによるSlot探索またはObject Type検査に失敗した．],
  [`3`], [`INVALID_DEPTH`], [Error値は定義されるが，主要なSlot探索経路では`INVALID_DESCRIPTOR`へ変換する．],
  [`4`], [`INVALID_ARGUMENT`], [Count，Index，Range，Address，Sizeのいずれかが受理されない．],
  [`5`], [`FATAL`], [HAL ErrorまたはKernel内部整合性Errorが発生した．],
  [`6`], [`DEBUG_UNIMPLEMENTED`], [宣言済みの操作またはTypeが実装されていない．],
)

`FATAL`からユーザ空間で回復する方法は定義されていない．`convert_kernel_to_capability_error()`は，すべての`kernel_error`を`FATAL`へ変換する．

== Concurrency and Lifetime

Capability ObjectとDependency列には，共通のLocking規約がない．SMP用のCPU Affinity FieldとLock Primitiveは存在するが，Capability操作全体のThread Safetyは保証されない．同じObjectやNodeを複数Coreから操作する場合は，User-levelのPolicyで直列化する必要がある．

Capability SlotのLifetimeは，Slotを所有するNode，またはProcess内部SlotのLifetimeに従う．Capability Object用のMemoryはGenericから単調増加で割り当てられ，`REMOVE`を行っても個別には返却されない．GenericをRevokeするとWatermarkが初期値に戻るため，派生Capabilityが残ったまま再割当てを行うとAliasが生じ得る．

== Reserved and Unavailable Types

#term[Virtualization Capability]群は，対象Revisionでは利用できない．Type値と一部のObject作成経路は存在するが，Virtual Machineを構成してGuestを実行する公開Interfaceは完成していない．

#reference_table(
  (1.65fr, 0.8fr, 1.3fr, 2.1fr),
  ([Object], [Type], [作成経路], [実装状態]),
  [#term[Virtual CPU]], [`13`], [Genericの`CONVERT`．], [`execute()`はOperationをDispatchせず，`ILLEGAL_OPERATION`を返す．],
  [#term[Virtual Address Space]], [`14`], [なし．], [Type値だけが予約され，Component実装を持たない．],
  [#term[Virtual Page Table]], [`15`], [なし．], [Generic変換は`INVALID_ARGUMENT`となり，Component実装を持たない．],
)

#notice(
  [WARNING],
  [Virtualization Capabilityを隔離境界として使用してはならない．Guest Memoryの隔離，Guest Entry，VM Exit，Virtual Interrupt，Revokeは公開Interfaceとして実装されていない．x86_64 HALに存在するVMX補助実装の状態は「x86_64 ABI」に記載する．],
)
