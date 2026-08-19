#import "/components/reference.typ" : reference_table, fields, notice, term
#import "@preview/cetz:0.5.2"

= Kernel Call

== Kernel Call Types

#term[Kernel Call]は，ユーザ空間からカーネルの機能を呼び出すための共通インターフェースである．命令，レジスタ配置，呼出しで破壊されるレジスタはアーキテクチャごとに異なる．カーネルが解釈するCall TypeとMessage Registerの意味は共通である．

A9Nが通常のUser-level Softwareへ提供するKernel Callは，`CAPABILITY_CALL`と`YIELD`の2つである．Capability Node，Memory，Process，IPC，Notification，Interrupt，I/Oを含むKernel Objectの操作は，すべて`CAPABILITY_CALL`へ集約される．Kernel Objectごとに独立したKernel Call Numberは存在しない．

#reference_table(
  (1fr, 0.7fr, 2.8fr),
  ([*Call*], [*Number*], [*Description*]),
  [`CAPABILITY_CALL`], [`-1`], [`MR0`のCapability Descriptorによって対象Capability Slotを探索し，`MR1`のObject Operationを実行する．],
  [`YIELD`], [`-2`], [実行中ProcessをReady Queueへ戻し，Quantumを10へ設定してSchedulingを実行する．Capabilityを要求しない．],
)

`DEBUG = -3`も実装に残るが，HeaderでDeprecatedとされる開発用Callである．`DEBUG`は通常のKernel Interfaceを構成するCallとして数えない．`MR0`の下位8 bitをKernel Loggerへ出力するだけであり，ProductionのUser-level Softwareは利用しない．

定義されていないCall Numberは，`INVALID_KERNEL_CALL` FaultとしてFault Dispatcherへ渡される．Resolver IPC Portを持たないProcessは`BLOCKED_SUSPEND`へ遷移する．

== Virtual Message Register

Virtual Message Registerは，Kernel Callの入力と出力を運ぶProcessごとの論理Word列である．各Wordは`MR0`，`MR1`，...のIndexで識別する．Virtual Message Register方式は，L4系MicrokernelのIPC設計に由来する@ElphinstoneEtAl:2013．KernelとKernel Objectは，HALの`get_message_register(process, index)`および`configure_message_register(process, index, value)`を介して同じIndex空間を読み書きする．

Architectureを$A$，Architecture ABIがHardware Registerへ割り当てるMR Indexの集合を$R_A$とする．HALが実装する保存先の写像$"map"_A(i)$は，次式で表せる．

$
  "map"_A(i) = cases(
    "hardware-context.register"_A(i) & i in R_A,
    "ipc-buffer.messages"[i] & i in.not R_A,
  )
$

Hardware Registerへ割り当てられるIndex集合$R_A$，各Indexに対応するRegister，Kernel Call Entryで破壊されるRegisterはArchitecture ABIが定める．Kernel Objectは保存先を区別せず，MR Indexだけを指定する．IPC PortもSenderの`MRi`からReceiverの`MRi`へ同じIndexでPayloadをCopyする．

#figure([
  #set text(size: 8pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(
      stroke: 0.45pt,
      mark: (transform-shape: false, fill: black),
    )
    scale(1.25)

    content((0, 0.35), [Kernel Objectが参照する論理Index], anchor: "center")

    let register-labels = (`MR0`, `MR1`, [$dots$], `MRi`, [$dots$], `MRn`)
    for (index, label) in register-labels.enumerate() {
      let x-left = -2.4 + index * 0.8
      let fill-color = if index == 3 { luma(225) } else { white }
      rect((x-left, 0), (x-left + 0.8, -0.55), fill: fill-color)
      content((x-left + 0.4, -0.275), label, anchor: "center")
    }

    content(
      (0.4, -1.4),
      [#align(center)[Architecture ABIによるMapping \ `map_A(i)`]],
      name: "mapping",
      frame: "rect",
      padding: 0.5em,
      fill: luma(240),
    )
    content(
      (-1.85, -3.05),
      [#align(center)[Hardware Context \ `register_A(i)`]],
      name: "hardware-context",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )
    content(
      (1.85, -3.05),
      [#align(center)[`IPC Buffer` \ `messages[i]`]],
      name: "ipc-buffer",
      frame: "rect",
      padding: 0.5em,
      fill: luma(247),
    )

    line((0.4, -0.55), "mapping.north", mark: (end: ">"))
    content((0.63, -0.8), [`get` / `configure`], anchor: "west")

    line(
      "mapping.south",
      (0.4, -2.2),
      (-1.85, -2.2),
      "hardware-context.north",
      mark: (end: ">"),
    )
    content((-1.15, -2.07), [$i in R_A$], anchor: "south")

    line(
      "mapping.south",
      (0.4, -2.2),
      (1.85, -2.2),
      "ipc-buffer.north",
      mark: (end: ">"),
    )
    content((1.15, -2.07), [$i in.not R_A$], anchor: "south")
  })
], caption: [Virtual Message Registerと保存先の対応])

Virtual Message Registerは型情報を持たない．Capability Descriptor，Operation Number，Bit Field，Address，Payloadの意味は，Kernel CallまたはCapability OperationごとのMessage Layoutが定める．呼出し側は，利用する最大MR IndexをArchitecture ABIとIPC Bufferの範囲に基づいて事前検査できる．事前検査の有無にかかわらず，Kernel Callの成否は返却されたResultから判定する．

== Capability Call

入力は次の通りである．

#fields(
  [`MR0`], [Raw Capability Descriptor．],
  [`MR1`], [対象ObjectのOperation Number．],
  [`MR2..`], [Operation固有の入力．],
)

出力は次の通りである．

#fields(
  [`MR0`], [成功時は1，失敗時は0．],
  [`MR1`], [失敗時の`capability_error`．],
  [`MR2..`], [Operation固有の出力．],
)

カーネルは，実行中ProcessのRoot Nodeを起点として，`MR0`のCapability Descriptorによって対象Capability Slotを探索する．空のSlot，探索途中のLeaf Object，Depthの不一致により対象Slotへ到達できない場合，`MR0 = 0`と`MR1 = INVALID_DESCRIPTOR`を返す．Message Registerとレジスタの対応は「x86_64 ABI」に記載する．

== IPC Buffer

#term[IPC Buffer]は，Virtual Message Registerの退避，IPCのPayload，Capability転送の情報に用いる共有データ構造である．大きさは1 Page以内である．Hardware Registerだけでは表せないVirtual Message RegisterをIPC Bufferへ割り当てるかどうかはHALが定める．

`messages`のWord数は，1 PageのWord数からCapability Transfer用の18 Wordを除いて算出する．16 WordはSource Descriptor列，2 WordはDestination Node DescriptorとDestination Indexである．

$ "MESSAGE_BUFFER_SIZE_MAX" = "PAGE_SIZE" / "sizeof(word)" - 18 $

#fields(
  columns: (1.7fr, 2.5fr),
  [`messages[MESSAGE_BUFFER_SIZE_MAX]`], [Virtual Message RegisterのBacking Store．MR Indexとの対応はArchitecture ABIが定める．],
  [`transfer_source_descriptors[16]`], [送信側Root Nodeから移動元Capability Slotを探索するためのDescriptorを格納する．],
  [`transfer_destination_node`], [受信側Root NodeからDestination Node Slotを探索するためのCapability Descriptorを格納する．],
  [`transfer_destination_index`], [受信側Destination Node内の開始Indexを格納する．],
)

PCBの`CONFIGURE`でIPC Buffer用Frameを設定すると，カーネルはFrame CapabilityをProcess内部のSlotへCopyし，Frameをカーネルから参照できるAddressへ変換して`process.buffer`へ設定する．InitにはBoot処理が同じ設定を行う．

#notice(
  [WARNING],
  [Hardware Registerへ割り当てられないMessage Registerを使う操作には，有効なIPC Bufferが必要である．IPC Bufferが設定されていない場合，HALは`NO_SUCH_ADDRESS`を返す．HAL Errorを`kernel_error`へ変換するCapability Operationは，Resultを`FATAL`として返す．Frameの大きさと配置はアーキテクチャごとのABIに従い，カーネルへ設定する前に対象ProcessのAddress SpaceへMapする必要がある．],
)

== Fault IPC

Process Control Blockへ#term[Fault Resolver]としてIPC Portを設定すると，KernelはProcess FaultをResolverへ`CALL`相当で配送する．Faultを起こしたProcessはReplyを受け取るまで`BLOCKED_FAULT`または`BLOCKED_REPLY`となる．Resolver Slotには`READ | WRITE`が必要である．

Fault Messageの共通Headerは次の通りである．`message_info.source`は`FAULT = 1`となる．

#fields(
  [`MR0`], [`1`．],
  [`MR1`], [`0`．],
  [`MR2`], [Kernelが生成した`message_info`．],
  [`MR3`], [Resolver Slot-local Identifier．],
  [`MR4`], [`fault_type`．],
)

#reference_table(
  (1.4fr, 2.8fr),
  ([*Fault Type*], [*Additional Message Registers*]),
  [`MEMORY`，`MEMORY_INSTRUCTION_FETCH`], [`MR5 = program counter`，`MR6 = fault address`，`MR7 = architecture fault code`．],
  [`INVALID_INSTRUCTION`], [`MR5 = program counter`，`MR6 = architecture fault code`．],
  [`INVALID_ARITHMETIC`], [`MR5 = program counter`，`MR6 = architecture fault code`．],
  [`INVALID_KERNEL_CALL`], [`MR5 = program counter`，`MR6 = kernel call number`，`MR7.. = hardware context`．Contextの長さと並びはアーキテクチャごとのABIが定める．],
  [`ARCHITECTURE`], [`MR5 = program counter`，`MR6 = architecture fault code`．],
)

`INVALID_KERNEL_CALL`へのReplyでは，Resolverから返されたHardware ContextがFaultを起こしたProcessへ適用される．Hardware Contextの書換えはPrivilege境界に影響するため，Resolverはアーキテクチャごとの制約を満たす値だけを返す必要がある．

== Interface Limitations

Capability Callは，不正なDescriptor，Operation，Rights，ArgumentをCapability Errorとして返す．ユーザ空間の呼出し層は，Descriptor，Message Length，Transfer Count，Destination IndexをKernel Callの前に追加検査できる．事前検査はErrorを早い段階で分類する手段であり，Kernelの検査を置き換える要件ではない．

Capability Callで成功時に定義される共通出力は`MR0`だけである．`MR1`は初期化されない．操作固有のResultを持たないCallでは，古いMessage Register値が残り得る．
