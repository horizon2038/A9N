#import "/components/reference.typ" : reference_table, notice, term
#import "@preview/cetz:0.5.2"

= Building Init and Services

== Scope and Preconditions

本章は，Init ImageをKernel Entryから起動し，Capabilityを管理し，最初のService Processを構成する手順を示す．読者は「Capability」，「Kernel Call」，Part IIIの使用対象Object，「Boot and Init Protocol」，「User-level System Architecture」を先に確認する必要がある．

手順はArchitectureに依存しないKernel Interfaceを用いる．Entry Stub，Register配置，Linker Script，Bootloaderの具体値はArchitecture ABIが定める．x86_64では「x86_64 ABI」を参照する．

== Init Image Requirements

「Boot and Init Protocol」はKernelがInit Imageへ要求する境界条件を定義する．Init Imageの実装では，少なくとも次の条件を満たす必要がある．

+ Bootloaderが読めるExecutable Imageであり，Entry Pointを持つ．
+ `init_info`を書き込む領域とIPC Buffer領域をImage内に確保する．
+ IPC Buffer領域を基本Pageの境界へ配置する．
+ Languageが生成するFunction Prologueより前に，有効なStack Pointerを設定する．
+ Link時に確定したAddressから`init_info`を取得する．
+ Entry Pointから戻らず，待機時は`YIELD`，IPC受信，Notification待機のいずれかを用いる．
+ Kernel Callの返却値を読み，Capability Errorを呼出し元へ返す．

`init_info`のPointerをEntry Point引数として受け取る共通規約はない．Init ImageのEntry StubがLinker Symbolを参照し，Language側のEntry PointへPointerを渡す．Stackと`init_info`の具体的なAddress，Alignment，RegisterはArchitecture ABIに従う．

== Kernel Call Adapter

Capability Operationの共通呼出し手順は次のように表せる．`enter_kernel`だけがArchitectureに依存する．`mr`はVirtual Message Registerを表す．

```text
capability_call(descriptor, operation, input):
    mr[0] = descriptor
    mr[1] = operation
    mr[2..] = input
    enter_kernel(CAPABILITY_CALL, mr)

    if mr[0] == 0:
        return error(mr[1])
    return operation_specific_output(mr)
```

成功時に共通で定義される値は`MR0 = 1`だけである．`MR1`は成功時に初期化されず，呼出し前のOperation Numberが残り得る．Adapterは，`MR0 = 0`の場合だけ`MR1`を`capability_error`として読む．`MR2..`はOperationが出力を定義する場合だけ読む．

不正なDescriptor，Operation，Rights，ArgumentはKernelがCapability Errorとして返す．Adapterに事前検査は要求されない．型付きAPIで早期にErrorを分類する場合は，次の値をKernel Call前に検査できる．

#reference_table(
  (1.3fr, 1.8fr, 1.5fr),
  ([*入力*], [*事前に判定できる内容*], [*Kernel Result*]),
  [Descriptor], [Encoded Depthと呼出し側台帳のSlot情報．], [`INVALID_DESCRIPTOR`または`INVALID_DEPTH`．],
  [Operation], [台帳上のCapability Typeと要求Rights．], [`ILLEGAL_OPERATION`または`PERMISSION_DENIED`．],
  [Message Layout], [Message Length，Transfer Count，最大MR Index．], [`INVALID_ARGUMENT`またはOperation固有Error．],
  [Address], [Page Size，Alignment，呼出し側が管理するMapping．], [`INVALID_ARGUMENT`またはOperation固有Error．],
)

Kernelが保持するCapability Slotの状態は，User-level Softwareから直接参照できない．事前検査後にCapability Stateが変化する可能性もある．Kernel Call後の成否は，事前検査の結果に関係なく，返却された`MR0`で確定する．

`YIELD`はCapability DescriptorとOperation Numberを取らない．呼出し層はArchitecture固有のKernel Call Entryへ`YIELD`のCall Numberを渡す．`YIELD`から復帰した時点では，別Processが実行された可能性を考慮し，共有MemoryやUser-level Resourceの状態を再確認する．

== Capability Inventory and Error Handling

Kernelは，Processから到達可能なCapabilityを列挙するOperationを提供しない．User-level Resource Managerは，自身が作成，移動，受信したCapabilityを台帳で追跡する．DescriptorはProcessのCapability Space内でのみ意味を持つため，別Processへ数値だけを渡しても同じCapabilityを参照できない．

#reference_table(
  (1.2fr, 1.3fr, 2.2fr),
  ([*Field*], [*例*], [*更新時点*]),
  [Descriptor], [`0x1000...`], [`CONVERT`，`COPY`，`MINT`，IPC受信の成功後に登録する．],
  [Type and Rights], [`IPC_PORT`，`READ | WRITE`], [作成元OperationとRights制限を記録し，`DEMOTE`後に更新する．],
  [Owner], [Init，Process Manager，Driver], [`MOVE`とCapability Transferの成功後に所有者を変更する．],
  [Dependency], [Source Slot，Generic], [`COPY`と`MINT`の派生関係，GenericからのObject作成を記録する．],
  [State], [Reserved，Active，Revoking], [多段階の作成と破棄中に同じSlotを再利用しないよう更新する．],
)

`COPY`はSourceとDestinationの両方を台帳へ残す．`MOVE`とCapability TransferはSourceを削除し，Destinationを登録する．`MINT`はSource Rightsを超えないRightsを持つ派生Entryを追加する．`REMOVE`と`REVOKE`は成功したSlotを台帳から削除する．複数Slotを変更するOperationが途中で失敗し得る場合，呼出し側は処理対象を`Revoking`等の中間状態へ移し，別の操作から隔離する．

Capability Errorは，少なくとも次の方針で上位処理へ伝播できる．

#reference_table(
  (1.4fr, 3fr),
  ([*Error*], [*User-level処理*]),
  [`ILLEGAL_OPERATION`], [Capability TypeとOperationの組合せ，またはObject Stateを確認する．同じ入力を無条件に再試行しない．],
  [`PERMISSION_DENIED`], [要求Rightsと使用したSlotを確認する．権限を追加せずに同じCallを再試行しない．],
  [`INVALID_DESCRIPTOR`，`INVALID_DEPTH`], [Capability台帳とDescriptor生成処理を確認する．失効済みEntryを削除する．],
  [`INVALID_ARGUMENT`], [Operation固有のCount，Index，Address，Bit Fieldを確認する．],
  [`FATAL`], [対象ProcessまたはServiceを隔離し，Kernel Logと直前のState遷移を保存する．],
  [`DEBUG_UNIMPLEMENTED`], [対象Operationを利用不可として扱い，別の実装経路を選択する．],
)

== Object Construction and Failure Recovery

Genericから複数Objectを作成してProcessを起動する処理は，単一のKernel Transactionではない．User-level Resource Managerは，各Operationの成功後にRollback情報を記録する．作成処理は，次の段階へ分割できる．

+ Destination Nodeと未使用Slotを予約する．
+ Genericの`CONVERT`で必要なKernel Objectを作成する．
+ Page TableとFrameをAddress SpaceへMapする．
+ CapabilityをChild Root Nodeへ配置し，Rightsを制限する．
+ PCBを`CONFIGURE`し，Entry，Stack，Address Space，Root Node，IPC Buffer，Fault Resolverを設定する．
+ 構成済みのPCBだけを`RESUME`する．

失敗時は，PCBを`RESUME`する前なら，作成順の逆順でMappingとCapability Slotを解除する．PCBを`RESUME`した後は，対象Processを停止し，IPC QueueとNotification Queueから外してからResourceを破棄する．Genericから割り当てた個別ObjectのMemoryは`REMOVE`だけではGenericへ戻らない．GenericのWatermarkを戻す`REVOKE`は，全派生Capabilityが失効したことをResource Managerが確認した場合だけ実行する．

#notice(
  [CAUTION],
  [PCBの`CONFIGURE`，Capability Transfer，Nodeの`REVOKE`は，途中まで状態を変更した後にErrorを返し得る．Errorを受け取っただけで呼出し前の状態へ戻ったと判断してはならない．],
)

== Bootstrap to a Service Process

Initは，受け取ったGenericとInitial Capability Spaceから，後続Processに必要なObjectを構成する．Executable LoaderとResource ManagerはUser-level Softwareとして実装する．最初のService Processを起動する順序は次の通りである．

+ `init_info`のVersion，`generic_list_count`，IPC Buffer Pointerを検査し，Root Slot 1から9をResource台帳へ登録する．
+ Genericを`CONVERT`し，Child用Capability Node，Address Space，Page Table，Frame，PCB，IPC Portを作成する．
+ Child用FrameをInit Address Spaceの一時領域へMapし，ExecutableのProgram Segmentと初期Dataを書き込む．
+ Child Address SpaceへPage TableとFrameをMapし，Entry Point，Stack，IPC BufferのUser Virtual Addressを確定する．
+ Child Root Nodeへ必要なCapabilityを`COPY`または`MINT`する．不要なRightsは`MINT`で除去する．
+ PCBの`CONFIGURE`でRoot Node，Address Space，IPC Buffer Frame，Fault Resolver，Entry Point，Stack Pointer，Priorityを設定する．
+ Serverを`RESUME`し，ServerがIPC Portで`REPLY_RECEIVE`を発行して待機した後にClientを`RESUME`する．
+ Init Address Spaceの一時Mappingと，配布を終えた一時Capabilityを解除する．

#figure([
  #set text(size: 7.5pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(
      stroke: 0.45pt,
      mark: (transform-shape: false, fill: black),
    )

    content((0, 0), [Generic], name: "generic", frame: "rect", padding: 0.65em, fill: luma(235))
    content((0, -1.3), [`CONVERT`], name: "convert", frame: "rect", padding: 0.65em, fill: luma(235))
    content((-3.7, -3.4), [Node], name: "node", frame: "rect", padding: 0.65em, fill: luma(247))
    content((-1.8, -3.4), [Address Space], name: "address-space", frame: "rect", padding: 0.65em, fill: luma(247))
    content((1.05, -3.4), [Page Table / Frame], name: "memory", frame: "rect", padding: 0.65em, fill: luma(247))
    content((3.9, -3.4), [PCB / IPC Port], name: "process", frame: "rect", padding: 0.85em, fill: luma(247))
    content((-2.4, -5.2), [Child Capability Space], name: "cspace", frame: "rect", padding: 0.65em, fill: luma(247))
    content((2, -5.2), [Child Address Space], name: "child-address-space", frame: "rect", padding: 0.65em, fill: luma(247))
    content((0, -7), [`PCB.CONFIGURE`], name: "configure", frame: "rect", padding: 0.65em, fill: luma(235))
    content((0, -8.5), [`PCB.RESUME`], name: "resume", frame: "rect", padding: 0.65em, fill: luma(235))

    line("generic.south", "convert.north", mark: (end: ">"))
    line("convert.south", (0, -2.35))
    line((-3.7, -2.35), (3.9, -2.35))
    for (x, target) in ((-3.7, "node"), (-1.8, "address-space"), (1.05, "memory"), (3.9, "process")) {
      line((x, -2.35), target + ".north", mark: (end: ">"))
    }
    line("node.south", "cspace.north", mark: (end: ">"))
    line("address-space.south", "child-address-space.north", mark: (end: ">"))
    line("memory.south", "child-address-space.north", mark: (end: ">"))
    line("cspace.south", (0, -6.3))
    line("child-address-space.south", (0, -6.3))
    line((0, -6.3), "configure.north", mark: (end: ">"))
    line("process.east", (5.15, -3.4), (5.15, -6.1), "configure.east", mark: (end: ">"))
    line("configure.south", "resume.north", mark: (end: ">"))
  })
], caption: [GenericからService Processを起動するまでの依存関係])

PCBを`RESUME`する時点で，Entry Point，Stack，Address Space，Root Nodeが相互に整合していなければならない．IPC Bufferを使うProcessでは，IPC Buffer FrameをChild Address SpaceへMapしてからPCBへ設定する．Fault Resolverを持たないProcessがFaultを起こすと，復帰手段を持たず`BLOCKED_SUSPEND`へ遷移する．開発中のProcessにもFault Resolverを設定する必要がある．

== IPC Service Implementation

IPC Protocolは，Port Capability，Slot-local Identifier，`message_info`，Payload Layout，Capability Transfer Layoutを一組として定義する．Operation NumberだけではRequest種別を表せないため，Service固有のMethod Numberを`MR4`以降のPayloadへ配置する．Serverは`message_length`を確認してから，各Methodが要求するWordを読む．

Service Managerは，ClientへIPC Port Capabilityを配布する前に，ClientまたはSessionごとのIdentifierを設定できる．Identifierを設定したSource Slotから，`MODIFY`を除いたRightsで`MINT`すれば，ClientはIdentifierを変更できない．ServerはKernelが`MR3`へ配送したIdentifierをClient，Session，Role，Request Route等へ対応付ける．Identifierの値域と再利用規則はService Protocolが定義する．

Clientの`CALL`は，Requestを送信した後にReplyまでBlockする．Server Loopは，開始時から`REPLY_RECEIVE`を利用できる．Reply Authorityを持たない最初の`REPLY_RECEIVE`ではReply部分が状態を変更せず，Receive部分が最初のRequestを待つ．

```text
server_loop(port):
    request = reply_receive(port, block = 1)

    loop:
        reply = dispatch(request.identifier, request.info, mr[4..])
        mr[4..] = encode(reply)
        request = reply_receive(port, block = 1)
```

`dispatch`は，Source Identifier，Message Length，Method Numberを組み合わせてRequestを分類する．Unknown Methodや短いPayloadには，Service固有のError Replyを返す．Kernelが返すCapability Errorと，Service Protocolが返すApplication Errorは別の型として扱う．

IPC Fastpathは，Serverが`REPLY_RECEIVE`を発行して`BLOCKED_RECEIVE`となった後に，Clientが`CALL`した場合に成立し得る．ServerはClientの`CALL`より先にReceiver Queueで待機する．各Requestの処理後はReplyを構成してから次の`REPLY_RECEIVE`へ入り，後続Clientを待つ．

PayloadのWord数がHardware Registerへ割り当てられた範囲を超える場合，残りのVirtual Message RegisterはIPC Bufferに置かれる．同じProcess内のKernel Call AdapterとIPC Protocol実装は，一つのIPC Bufferを共有する．Nested CallやCallbackを実装する場合は，外側のRequestを別のUser Memoryへ退避してからMessage Registerを再利用する．

Capability Transferを行うSenderは，Source Descriptor列と`transfer_count`を設定する．Receiverは，Destination Node Descriptorと開始Indexを設定してから受信する．TransferはMoveであるため，成功後のSenderはSource Capabilityを利用できない．Protocolは，移動するCapability Type，Rights，個数，Destination Slotの所有者をMethodごとに定義する．

== Notification and Fault Resolver

Notification Portは，回数ではなくPending Bit集合を配送する．Driver Managerは，通知側へCapabilityを配布する前にSlot-local Identifierを設定し，一つのBitを一つのEvent Sourceへ割り当てられる．Driverは，`WAIT`または`POLL`で得たWordをBitごとに処理する．同じBitへ複数回通知しても一つにまとまるため，Event回数が必要なProtocolは，共有Memory上のCounterまたはDevice StateをNotification受信後に読む．

Fault Resolverは，対象ProcessのPCBへ設定したIPC PortでFault Messageを受け取るServiceである．ResolverはFault Type，Program Counter，Fault Address，Architecture Fault Codeを読み，次のいずれかを選択する．

+ Missing Mappingを構成し，Faultを起こしたProcessへReplyする．
+ User Contextを規約内の値へ修正してReplyする．
+ 回復不能Faultとして対象Processを停止し，Resource Managerへ通知する．

ResolverからのReplyは，Faultを起こしたProcessのHardware Contextを変更し得る．Resolverは一般Clientより強いAuthorityを持つため，専用Processへ分離し，Resolver用IPC PortをClientへ配布しない．Resolver自身のAddress Space，Stack，IPC Bufferは，Resolverが処理するProcessへ依存させない．

#figure([
  #set text(size: 7.5pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(
      stroke: 0.45pt,
      mark: (transform-shape: false, fill: black),
    )

    content((0, 1.4), [Init / Resource Manager], name: "manager", frame: "rect", padding: 0.65em, fill: luma(235))
    content((-4.6, 0), [Client], name: "client", frame: "rect", padding: 0.65em, fill: luma(247))
    content((-1.55, 0), [IPC Port], name: "ipc", frame: "rect", padding: 0.65em, fill: luma(247))
    content((1.55, 0), [System Service], name: "service", frame: "rect", padding: 0.65em, fill: luma(247))
    content((4.6, 0), [Fault Resolver], name: "resolver", frame: "rect", padding: 0.65em, fill: luma(247))
    content((-1.55, -2), [Interrupt Port], name: "interrupt", frame: "rect", padding: 0.65em, fill: luma(247))
    content((1.55, -2), [Notification Port], name: "notification", frame: "rect", padding: 0.65em, fill: luma(247))

    line("manager.south", (-4.6, 0.9), "client.north", mark: (end: ">"))
    line("manager.south", (1.55, 0.9), "service.north", mark: (end: ">"))
    line("client.east", "ipc.west", mark: (end: ">"))
    line("ipc.east", "service.west", mark: (end: ">"))
    line("service.east", "resolver.west", mark: (end: ">"))
    line("interrupt.east", "notification.west", mark: (end: ">"))
    line("notification.north", "service.south", mark: (end: ">"))
  })
], caption: [User-level Serviceを構成するCapability経路])
== Service Shutdown

SMP BuildのGiant Lockは，個々のKernel Callに含まれるCapability ObjectとWait Queueの更新をCore間で直列化する．User-level System全体を覆い，複数Kernel CallをAtomicにまとめるTransactionは存在しない．同じNode，Generic，IPC Port，Notification Port，PCBのLifecycleを複数Processから変更する場合，Resource ManagerはObject単位でOperation列を直列化する．Operationの完了後に台帳を更新し，台帳更新前のObjectを別のWorkerへ渡さない．

Serviceを停止する場合は，次の順序でResourceを閉じる．

+ 新しいClientへService Capabilityを配布しない．
+ Client Requestの受付を停止し，Reply待ちとWait Queueを解消する．
+ PCBを停止し，Interrupt PortとNotification PortのBindingを解除する．
+ Child Address SpaceからFrameとPage TableをUnmapする．
+ Child Root Node内のCapabilityを`REVOKE`または`REMOVE`する．
+ 全派生Capabilityの失効を確認した後に，親GenericをRevokeする．

IPC PortのRevokeはWaiterを自動的に起こさず，GenericのRevokeは派生Object MemoryをClearしない．破棄順序はKernel Callの成功だけでなく，User-level台帳と各ProcessのStateを用いて確認する．

== Verification

Build成功だけでは，User-level SoftwareがA9N上で動作したとは判定しない．検証は，失敗箇所を分離できる順序で進める．

#reference_table(
  (1.1fr, 1.6fr, 2.7fr),
  ([段階], [合格条件], [失敗時の確認箇所]),
  [Image], [Entryと予約領域を機械的に確認できる．], [Linker Script，Load Segment，Symbol Table，Alignmentを確認する．],
  [Boot], [LoaderがKernelとInitを配置する．], [`boot_info`，Image Path，Frame数，Entry Pointを確認する．],
  [Init Entry], [User Entryの識別文字列を観測できる．], [Stack，`init_info` Address，Kernel Call Entryを確認する．],
  [Capability Call], [成功例と失敗例が期待したResultを返す．], [Descriptor，Rights，Operation Number，Message Layoutを確認する．],
  [Capability Inventory], [Copy，Move，Mint，Transfer後のOwnerとRightsが台帳と一致する．], [Source Slot，Destination Slot，Dependency，途中失敗時の中間Stateを確認する．],
  [Process], [ChildがEntryへ到達し，Faultを配送できる．], [Address Space，Frame，PCB，Fault Resolver，Scheduling Stateを確認する．],
  [IPC], [Serverが待機後にClient Messageを受信してReplyする．], [Message Info，Queue State，Reply Authority，Priorityを確認する．],
  [Notification], [複数Bitと同一Bitの再通知が定義したEvent処理になる．], [Identifier，Pending Bit，Device Stateの読出しを確認する．],
  [Teardown], [Waiterを残さず，派生Capabilityを失効してResourceを閉じる．], [停止順，Unmap，Binding解除，Revoke対象を確認する．],
)

各段階では，成功経路と少なくとも一つの意図的な失敗経路を実行する．Capability Callの失敗経路では，存在しないDescriptorまたは不足したRightsを与え，`MR0 = 0`と期待する`capability_error`を確認する．IPCの失敗経路では，Message LengthとCapability Transfer数を上限で検査する．

`DEBUG` Kernel Callは，Init Entryまでの起動確認に限って利用する．`DEBUG`はDeprecatedであり，Applicationの出力Interfaceとして扱わない．Systemの出力は，権限を制限したI/O Serviceを構成し，ClientからIPCで利用する．
