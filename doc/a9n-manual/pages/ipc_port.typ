#import "/components/reference.typ" : reference_table, operations, fields, notice, term
#import "@preview/bytefield:0.0.7": *
#import "@preview/cetz:0.5.2"

= IPC Port

== Object Model

#term[IPC Port]は，MessageとCapabilityをProcess間で転送するCapability Objectである．PortはSender QueueまたはReceiver QueueをFIFOで持つ．Port Stateは`WAIT`，`READY_TO_SEND`，`READY_TO_RECEIVE`のいずれかであり，SenderとReceiverが同時にQueueへ入ることはない．

IPC Port Capability Slotは，Slot-local Identifierを`data[0]`に持つ．同じIPC Port Objectを参照するSlotであっても，Identifierは個別に設定できる．Receiverは，送信に使われたSlotのIdentifierを`MR3`で受け取る．`MR3`はSenderのPayloadではなく，KernelがSlot-local Dataから設定する．

== Message Layout

全IPC操作は，`MR2`を`message_info`として読み取る．ユーザ空間から呼び出す場合は，`source = NORMAL`を設定する．別のSourceを指定すると，カーネルは`INVALID_ARGUMENT`を返す．

#reference_table(
  (1fr, 1.2fr, 2.6fr),
  ([*Bits*], [*Field*], [*Description*]),
  [`0`], [`block`], [Peer不在時にProcessをBlockする場合は1，Blockせずに戻る場合は0．],
  [`1..8`], [`message_length`], [`MR4`から始まるPayload Word数．範囲は0から255．],
  [`9..12`], [`transfer_count`], [移動するCapability数．範囲は0から15．],
  [`13..14`], [`source`], [`NORMAL = 0`，`FAULT = 1`，`NOTIFICATION = 2`，`RESERVED = 3`．],
  [`15`], [`reserved`], [Kernelは明示検査しない．ユーザ空間は0を設定する必要がある．],
  [`16..63`], [未使用．], [Kernelは解釈しない．ユーザ空間は0を設定する必要がある．],
)

#figure(
  bytefield(
    bpr: 16,
    msb: left,
    rows: (9em),
    bitheader(
      "bounds",
      0,
      1,
      9,
      13,
      15,
      text-size: 8pt,
    ),
    flag[RESERVED],
    bits(2)[SOURCE],
    bits(4)[TRANSFER COUNT],
    bits(8)[MESSAGE LENGTH],
    flag[BLOCK],
    text-size: 4pt,
  ),
  caption: [`message_info`のBit配置],
)

#fields(
  [`MR0`], [IPC Port Capability Descriptor．],
  [`MR1`], [Operation Number．],
  [`MR2`], [`message_info`．`IDENTIFY`では新しいIdentifierと兼用する．],
  [`MR3`], [受信時のSource Identifier．送信時の値はPayloadとして転送されない．],
  [`MR4..`], [`message_length` WordのPayload．],
)

Payloadは，SenderとReceiverの同じMR Index間で転送する．`message_length = n`の場合は，`MR4`から`MR(3+n)`までを転送する．Payloadがレジスタに割り当てられたMRを超える場合は，IPC Bufferが必要となる．

== Operation Summary

#operations(
  [`SEND = 1`], [`MR2 = info` \ `MR4.. = payload`], [MessageをReceiverへ転送する．IPC Port Slotに`WRITE`を要求する．Receiver不在かつ`block = 1`の場合，Senderは`BLOCKED_SEND`となる．], [`PERMISSION_DENIED`，`INVALID_ARGUMENT`，`FATAL`．],
  [`RECEIVE = 2`], [`MR2 = info`], [SenderからMessageを受信する．IPC Port Slotに`READ`を要求する．Sender不在かつ`block = 1`の場合，Receiverは`BLOCKED_RECEIVE`となる．既存Reply Authorityを破棄する．], [`PERMISSION_DENIED`，`INVALID_ARGUMENT`，`FATAL`．],
  [`CALL = 3`], [`MR2 = info` \ `MR4.. = payload`], [Messageを送り，選択されたReceiverからのReplyを待つ．IPC Port Slotに`WRITE`を要求する．], [`PERMISSION_DENIED`，`INVALID_ARGUMENT`，`FATAL`．],
  [`REPLY = 4`], [`MR2 = info` \ `MR4.. = payload`], [Processが保持するReply AuthorityへMessageを返す．IPC Port Slot Rightsを要求しない．Reply Authorityがない場合も，状態を変更せず成功を返す．], [`INVALID_ARGUMENT`，`FATAL`．],
  [`REPLY_RECEIVE = 5`], [`MR2 = info` \ `MR4.. = reply payload`], [Replyを完了してから同じPortで次のMessageを受信する．IPC Port Slotに`READ`を要求する．], [`PERMISSION_DENIED`，`INVALID_ARGUMENT`，`FATAL`．],
  [`IDENTIFY = 6`], [`MR2 = identifier`], [呼出しに使用したIPC Port SlotのIdentifierを変更する．IPC Port Slotに`MODIFY`を要求する．], [`PERMISSION_DENIED`，`INVALID_ARGUMENT`，`FATAL`．],
)

== SEND and RECEIVE

Receiverが待機している場合，`SEND`はQueue先頭のReceiverへMessageを転送し，ReceiverをReady Queueへ戻す．Senderは実行を続ける．Senderが待機している場合，`RECEIVE`はQueue先頭のSenderからMessageを受け取り，SenderをReady Queueへ戻す．

`RECEIVE`を実行するProcessがReply Authorityを持っている場合，カーネルは古いClientとのReply Linkを解除する．ReplyせずにClientを切り離す場合は`RECEIVE`を，Replyを完了してから次のMessageを待つ場合は`REPLY_RECEIVE`を用いる．

Peerが存在せず，`block = 0`の場合，操作は成功を返すがMessageを転送しない．対象Revisionは，Non-blocking操作でもPort Stateを`READY_TO_SEND`または`READY_TO_RECEIVE`へ変更する．Queueが空のままStateだけが変わるため，後から呼ばれたPeer側の操作が`FATAL`を返し，Stateを`WAIT`へ戻す経路がある．対象Revisionでは，Non-blocking IPCをPollに使用してはならない．

== CALL and Reply Authority

Receiverが待機している場合，`CALL`はCallerを`BLOCKED_REPLY`へ移す．カーネルはCallerに`source_reply_target`を，Receiverに`destination_reply_target`を設定する．Reply Authorityを使えるのはReceiverだけである．Reply AuthorityはProcess Stateであり，Capability SlotとしてCopyまたはTransferできない．

`REPLY`はReply Authorityが指すClientへPayloadとCapabilityを転送し，ClientをReady Queueへ戻す．ServerのReply Stateは`NONE`となる．実装はClientへ直接切り替えず，Clientを後続のScheduling対象とする．

`REPLY_RECEIVE`は，Reply PayloadをClientへ転送してReady Queueへ戻した後，次のSenderから受信する．Server Loopでは，最初に`RECEIVE`を呼び，以後は`REPLY_RECEIVE`を繰り返せる．

Peerが存在せず，`block = 0`の`CALL`はReply待ち状態へ入らず，成功を返す．Receiverが待機中の場合は，`block`値に関係なくCallerがReplyまでBlockされる．

== IPC Fastpath

#term[IPC Fastpath]は，Serverが`REPLY_RECEIVE`を発行して`BLOCKED_RECEIVE`となった後，Clientの`CALL`によってClientからServerへ直接切り替える実行経路である．成立条件は，`CALL`の実行時点で，(1) 対象ServerがReceiver Queueで待機していること，(2) Serverより高いPriorityの実行可能ProcessがReady Queueに存在しないことである．Direct Process SwitchとReply／Receive統合は，L4系Microkernelで用いられてきたIPC Optimizationに由来する@ElphinstoneEtAl:2013．IPC Fastpathは独立したOperation Numberを持たない．

Fastpath成立時は，対象ServerによるRequest処理を同じPriority以下のReady Processより先に実行し，通常の`schedule()`呼出しとReady Queueの探索を省略する．短縮されるのは，Clientの`CALL`からServerがRequest処理を開始するまでのScheduling経路である．

#figure([
  #set text(size: 8pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(
      stroke: 0.45pt,
      mark: (transform-shape: false, fill: black),
    )
    scale(1)

    for (x, label, participant-name) in (
      (-4.8, [Client], "client"),
      (-1.6, [`IPC Port`], "ipc-port"),
      (1.6, [Server], "server"),
      (4.8, [Scheduler], "scheduler"),
    ) {
      content(
        (x, 0.25),
        label,
        name: participant-name,
        frame: "rect",
        padding: 0.5em,
        fill: luma(247),
      )
      line(
        participant-name + ".south",
        (x, -10.7),
        stroke: (paint: gray, thickness: 0.4pt, dash: "dashed"),
      )
    }

    line((1.6, -0.9), (-1.6, -0.9), mark: (end: ">"))
    content((0, -0.72), [`REPLY_RECEIVE(block = 1)`], anchor: "south")
    content(
      (-1.6, -1.4),
      [#align(center)[Receiver Queue \ Server]],
      name: "receiver-queue",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )
    content(
      (1.6, -1.4),
      [`BLOCKED_RECEIVE`],
      name: "server-blocked",
      frame: "rect",
      padding: 0.5em,
      fill: luma(240),
    )

    line((-4.8, -2.4), (-1.6, -2.4), mark: (end: ">"))
    content((-3.2, -2.22), [`CALL` / Request], anchor: "south")
    content(
      (-4.8, -2.85),
      [`BLOCKED_REPLY`],
      name: "client-blocked",
      frame: "rect",
      padding: 0.5em,
      fill: luma(240),
    )

    line((-1.6, -3.15), (1.6, -3.15), mark: (end: ">"))
    content((0, -2.97), [Request / Capability], anchor: "south")
    content(
      (1.6, -3.7),
      [#align(center)[Reply Authority → Client \ Server: `READY`]],
      name: "server-ready",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )

    line((-1.6, -4.95), (4.8, -4.95), mark: (end: ">"))
    content((1.6, -4.77), [`try_direct_schedule(Server)`], anchor: "south")

    rect((-1.2, -5.3), (6.15, -10.7), stroke: 0.45pt)
    content(
      (1.25, -5.55),
      [#align(center)[*Fastpath* \ Serverより高い \ PriorityのProcessなし]],
      anchor: "north",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )
    content(
      (4.6, -5.55),
      [#align(center)[通常の`schedule()` \ 呼出しを省略]],
      anchor: "north",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )
    line((4.8, -7.05), (1.6, -7.05), mark: (end: ">"))
    content(
      (3.2, -7.22),
      [Serverを優先選択],
      anchor: "north",
    )
    rect((1.5, -7.05), (1.7, -7.32), fill: luma(225))

    line((-1.2, -7.7), (6.15, -7.7), stroke: 0.45pt)
    content(
      (1.25, -8.45),
      [#align(center)[*Fallback* \ Serverより高い \ PriorityのProcessあり]],
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )
    content(
      (4.6, -8.45),
      [#align(center)[Server \ Ready Queue \ へ追加]],
      name: "fallback-enqueue",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )
    content(
      (4.6, -9.95),
      [#align(center)[`schedule()` \ 最高Priority \ Processを選択]],
      name: "fallback-select",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )
    line("fallback-enqueue.south", "fallback-select.north", mark: (end: ">"))
  })
], caption: [`REPLY_RECEIVE`によるServer待機を前提とする`CALL` Fastpath])

Server Loopは最初のRequestを`RECEIVE`で受信し，Reply Authorityを得た後は`REPLY_RECEIVE`を繰り返す．`REPLY_RECEIVE`は旧ClientへReplyを転送した後，SenderとPending Notificationが存在せず，`block = 1`ならServerを`BLOCKED_RECEIVE`としてReceiver Queueへ追加する．CALL + `REPLY_RECEIVE` Fastpathでは，Serverの待機がClientの`CALL`より先に発生する．

#block(breakable: false)[
  後続の`CALL`はReceiver QueueからServerを取り出し，Clientを`BLOCKED_REPLY`へ設定して，ServerとのReply Linkを構成する．カーネルはRequestのVirtual Message RegisterとCapabilityを転送し，転送完了後にServerを`READY`へ設定する．Serverより高いPriorityの実行可能Processが存在しなければ，Direct ScheduleによってClientからServerへ切り替える．高いPriorityのProcessが存在する場合は，通常のSchedulingがReady Queue内の最高Priority Processを選択する．
]

`REPLY_RECEIVE`の発行時点でSenderがすでに待機している場合，カーネルは旧ClientへのReplyとQueue先頭のSenderからの受信を同じKernel Call内で行う．次Senderが`CALL`を実行していた場合は，ServerのReply Authorityを次Clientへ付け替える．Sender待機時のReply／Receive統合経路ではServerが実行を継続するため，旧ClientへのContext SwitchとServerの再選択は発生しない．

IPC PortにSenderが存在しない場合，`REPLY_RECEIVE`はBind済みNotificationを確認する．Pending Notificationも存在せず，`block = 1`ならServerは`BLOCKED_RECEIVE`となり，Schedulerが次Processを選択する．`block = 0`ならServerはBlockせずに戻る．IPC FastpathはMessage CopyとCapability Transferを省略しない．`message_length = 0`かつ`transfer_count = 0`の場合だけ，Payload CopyとCapability Transferを省略する．

IPC OperationはTimeoutを受け取らない．`BLOCKED_REPLY`のClientはReplyが完了するまで自動ではReadyにならない．ServerまたはClientを破棄する場合，Process ManagerはReply Linkを解消してからPCBをRevokeする必要がある．

== Capability Transfer

Capability TransferはCopyではなくMoveである．SenderはIPC Bufferの`transfer_source_descriptors[i]`にSource Descriptorを設定する．Receiverは`transfer_destination_node`にDestination Node Descriptorを，`transfer_destination_index`に開始Indexを設定する．転送数はSenderが`message_info.transfer_count`へ設定する．

SenderとReceiverの両方に，有効な`process.buffer`と`buffer_frame.type == FRAME`が必要である．Destination DescriptorによってReceiverのRoot NodeからDestination Node Slotを，Source DescriptorによってSenderのRoot NodeからSource Slotを探索する．Destination Node SlotのCapabilityは`NODE`を指し，Capabilityの移動先となるDestination Slotは`NONE`でなければならない．

TransferはIndex順に行う．途中でErrorが起きても，移動済みのCapabilityは元に戻らない．Source SlotとDestination NodeのRightsは検査しない．IPC Portへの送信権限を持つCodeは，SenderのRoot Capability Nodeから到達できるCapabilityを移動できる．Transfer Metadataを作成するCodeを信頼対象に含める必要がある．

#notice(
  [WARNING],
  [Destination IndexとNode Slotの範囲は検査されない．開始Indexを$i$，Node Slot数を$n$，転送数を$c$とすると，Receiverは$i < n$かつ$c <= n - i$をTransfer前に検証する必要がある．条件を満たさないTransferはKernel Memoryを破壊し得る．],
)

== IDENTIFY

`IDENTIFY`は`MR2`を新しいSlot-local Identifierとして保存する．IPC Portの共通Dispatchは`MR2`を先に`message_info`として検査するため，Identifierのbit 13..14は0でなければならない．任意のWord値をIdentifierとして使用することはできない．

`SEND`または`CALL`を実行すると，Kernelは呼出しに使用されたSlotのIdentifierをProcess Stateへ保持する．Receiverが既に待機している場合も，SenderがQueueで待機した後に受信される場合も，Kernelは保持したIdentifierをReceiverの`MR3`へ書く．Senderが`MR3`へ置いた値はMessage Payloadとして転送されないため，Receiverは`MR3`をMessageの送信に使用されたCapability Slotと対応する値として扱える．

Capabilityの`COPY`と`MINT`はIdentifierを複製する．`IDENTIFY`で一方のSlotを変更しても，別SlotのIdentifierは変化しない．Service Managerは，Clientごとに異なるIdentifierを設定したIPC Port Capabilityを配布することで，Server側のClient識別，Session選択，Request Routing，Accounting等へ利用できる．識別に用いるSlotから`MODIFY`を除かなければ，ClientはIdentifierを変更できる．

== Concurrency and Revoke

IPC Port QueueとReply Stateに対する共通Lockは存在しない．同じIPC Portを複数Coreから操作する場合は，ユーザ空間で直列化する必要がある．

IPC Portの`revoke()`は空実装であり，Wait Queue内Processを解除しない．IPC Portを破棄する前に，待機中Sender，Receiver，Reply待ちClientを解放する必要がある．PCBの`SUSPEND`とRevokeは，Processが所属するIPC Wait QueueからProcessを外す．
