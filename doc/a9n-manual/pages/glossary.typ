#import "/components/reference.typ" : term

#set heading(numbering: none)
#set par(first-line-indent: 0pt)

#let entry(name, description) = block(
  width: 100%,
  breakable: false,
  below: 0.9em,
)[
  #stack(
    dir: ttb,
    spacing: 0.25em,
    term[*#name*],
    pad(top: 0.5em, left: 1.5em,)[#description],
  )
]

= Glossary

== Architecture-independent Terms

#entry([Access Control List], [Objectごとに，呼出し主体のIdentityと許可するOperationを対応付けたEntryを保持するAccess Control Modelである．ACLと略記する．])

#entry([A9N Microkernel], [Capabilityに基づく権限管理を採用した第3世代のMicrokernelである．Kernel Objectの操作，Processの実行，仮想メモリ，IPC，Notification，割り込み配送の機構を提供する．])

#entry([A9N Ecosystem], [A9N Kernelを用いるSystemのBuild，Boot，User-level Runtime，Kernel Call Adapter，共有型を構成するSoftware群である．])

#entry([a9n_abi], [Kernel Call Number，Capability Operation Number，Message Register Layout，ArchitectureごとのKernel Call Wrapperを提供する`no_std` Rust crateである．])

#entry([a9n_types], [Capability，Message，Init，IPC Bufferに関する共有値を定義する，アーキテクチャ非依存の`no_std` Rust crateである．])

#entry([Base Library], [Freestanding環境でKernelとUser-level Systemが使用する基盤Libraryである．Container，Result，Option，libc互換処理を提供する．])

#entry([Capability], [Kernel Objectに対する操作権限を表す，カーネル管理の参照である．Object Type，Rights，Slot-local Data，Dependency情報とともにCapability Slotへ格納する．])

#entry([Capability Call], [Capability Descriptorによって対象Capability Slotを探索し，指定したKernel Object Operationを実行するKernel Callである．])

#entry([Capability Descriptor], [Root Capability Nodeを起点としてCapability Slotを探索するための1 Word Addressである．上位8 bitにEncoded Depthを，残りのbitにDescriptor Payloadを格納する．])

#entry([Capability Space], [Root Capability Nodeから到達できるCapability NodeとCapability Slotの階層である．])

#entry([Capability Slot], [Capabilityを格納するカーネル管理領域である．Objectへの参照，Type，Rights，3 WordのSlot-local Data，Dependency情報を保持する．])

#entry([Depth], [Capability Descriptor先頭から，Slot探索を終了するNode境界までのbit数である．8 bitのEncoded Depth Fieldと，探索経路上で消費するIgnore BitsおよびRadix Bitsの総和で表す．])

#entry([Direct Schedule], [IPCの通信相手を通常のSchedulingを介さずに選択する経路である．Ready Queueに通信相手より高いPriorityのProcessが存在する場合は，通常のSchedulingへ戻る．])

#entry([Giant Lock], [Kernel Entry全体を一つのLockで直列化するSMP構成である．A9NのSMP BuildはKernel Call，Interrupt，Timer，IPI，Fault Handlerの先頭で共通Lockを取得する．])

#entry([Descriptor Payload], [Capability Descriptorの下位$W - 8$ bitである．各Capability NodeのNode Indexを上位側から順に配置する．])

#entry([Encoded Depth], [Capability Descriptorの上位8 bitへ格納する値である．Depthを$D$とすると，Encoded Depthは$D - 8$となる．])

#entry([HAL], [Hardware Abstraction Layerの略称である．CPU初期化，Context Switch，Memory Mapping，割り込み制御，Kernel Call Entryのアーキテクチャ依存処理を共通Kernel Interfaceへ接続する．])

#entry([Ignore Bits], [Capability Nodeの探索時に読み飛ばすDescriptor bit数である．])

#entry([Kernel Call], [ユーザ空間からカーネル機能を呼び出す共通インターフェースである．通常利用するCallは`CAPABILITY_CALL`と`YIELD`の2つである．Deprecatedな`DEBUG`も実装に残る．])

#entry([Kernel Call Adapter], [Operation固有の値をVirtual Message Registerへ配置し，ArchitectureごとのKernel Call Entryを呼び出し，結果をUser-level Softwareの値へ戻す層である．])

#entry([Kernel Object], [Capabilityを介して操作するカーネル管理Objectである．ユーザ空間へObjectのMemory Addressを公開しない．])

#entry([Policy/Mechanism Separation], [機構と方針の分離．Resourceの操作と保護に必要なMechanismをKernelへ置き，Resourceの配分やServiceの構成を決めるPolicyをUser-level Softwareへ委ねる設計原則である．])

#entry([Nun], [A9N上でRust製User-level Softwareを構築するための`no_std` Runtimeである．Entry生成，IPC Buffer初期化，Debug出力，Panic処理を提供する．])

#entry([Virtual Message Register], [Kernel Callの引数と結果を運ぶProcessごとの論理Word列である．Message RegisterまたはMRと略記する．HALが各IndexをHardware Context内のRegisterまたはIPC Bufferへ割り当てる．])

#entry([Node Index], [Capability Node内のCapability Slotを選択するIndexである．DescriptorからRadix Bits分を取り出して算出する．])

#entry([Object-Capability Model], [Objectを指定する参照とAuthorityをCapabilityとして一体化し，呼出し主体から到達可能なCapabilityを操作可否の根拠とするAccess Control Modelである．])

#entry([Radix Bits], [Capability NodeのSlot数とNode Indexの幅を表すbit数である．Nodeは$2^"radix_bits"$個のSlotを持つ．])

#entry([Reply Authority], [IPCの`CALL`に対してReplyできる一時的な権限である．Process Stateとして保持し，Capability SlotとしてCopyまたはTransferできない．])

#entry([Rights], [Capability Slotに設定する操作権限のbit集合である．`READ`，`WRITE`，`COPY`，`MODIFY`を定義するが，具体的な検査内容はOperationごとに異なる．])

#entry([Slot-local Data], [Capability Slotごとに保持する3 Wordの補助情報である．IPC PortとNotification PortのIdentifier，I/O Port CapabilityのAddress Rangeに使用する．])

#entry([Slot-local Identifier], [IPC PortまたはNotification Portを参照するCapability Slotの`data[0]`に保持する1 Word値である．Kernelは，IPCではReceiverの`MR3`へ，NotificationではPending Flagを介して`MR2`または`MR3`へ配送する．])

#entry([Symmetric Multiprocessing], [複数のCPU Coreが同じKernelとMemoryを共有してUser Processを並行実行する構成である．SMPと略記する．A9NではBuild時に有効化する．])

#entry([User-level System], [Initを起点としてユーザ空間に構成するOS部分である．Capabilityの配布，Memory Manager，Driver，File System，System ServiceのPolicyを実装する．])

#entry([User-level Software], [A9NのUser Modeで動作し，Kernel CallによってKernel Objectを操作するSoftwareである．Initと，Initが構成するService Processを含む．])

#entry([Word], [A9NのKernel Interfaceが整数，Address，Capability Descriptor，Message Registerの基準に用いる`a9n::word`である．32-bit Wordは4 Byte，64-bit Wordは8 Byteである．])

== Kernel Objects and Runtime

#entry([Address Space], [Root Page Tableを表すCapability Objectである．Page TableとFrameのMappingおよびUnmappingを実行する．])

#entry([A9NLoader-rs], [A9N Boot Protocolに従ってKernel ImageとInit Imageを配置し，`boot_info`をKernelへ渡すUEFI Bootloaderである．])

#entry([Application Processor], [SMP SystemでBootstrap Processor以外のCPU Coreである．APと略記する．BSPからINIT IPIとStartup IPIを受けて起動する．])

#entry([Capability Node], [$2^"radix_bits"$個のCapability Slotを持つRadix Tree Nodeである．別のCapability NodeをSlotへ格納してCapability Spaceを階層化できる．])

#entry([Fault Resolver], [Process Faultの配送先としてPCBへ設定するIPC Portである．ResolverはFault Messageを受信し，必要に応じてReplyする．])

#entry([Frame], [物理メモリ範囲を表すCapability Objectである．Address Space Operationの引数としてVirtual AddressへMapする．])

#entry([Generic], [Kernel Objectの作成に使用する物理メモリ領域を表すCapability Objectである．Base Address，Size Bits，Device Flag，Watermarkを保持する．])

#entry([Init], [カーネルが最初に起動するUser Processである．起動時にInitial Capability Space，Address Space，Hardware情報を受け取る．])

#entry([Init Image], [Bootloaderが配置し，Kernelが最初のUser ProcessとしてMapするExecutable Imageである．Entry Point，`init_info`領域，IPC Buffer，Stackを含む．])

#entry([Interrupt Port], [一つのIRQ番号を保持し，割り込みをBind済みNotification Portへ配送するCapability Objectである．])

#entry([Interrupt Region], [IRQ番号を割り当てる権限を表すCapability Objectである．未使用IRQに対応するInterrupt Portを作成する．])

#entry([I/O Port Capability], [HALが提供するI/O SpaceへのRead／Write権限を表すKernel Objectである．I/O Addressの意味とAccess方法はHALが定める．])

#entry([IPC Buffer], [Message Registerの退避，IPC Payload，Capability TransferのMetadataに用いる共有データ構造である．])

#entry([IPC Fastpath], [Serverが`REPLY_RECEIVE`を発行してReceiver Queueで待機した後，Serverより高いPriorityの実行可能Processが存在しない場合に，Clientの`CALL`からServerへDirect ScheduleするIPC経路である．])

#entry([IPC Port], [MessageとCapabilityをProcess間で転送するCapability Objectである．Sender QueueまたはReceiver QueueをFIFOで保持する．])

#entry([Notification Port], [Word幅のPending FlagとFIFOのWait Queueを持つCapability Objectである．通知値をBitwise ORで蓄積する．])

#entry([Page Table], [Address Translationの中間Tableを表すCapability Objectである．Mapping操作はAddress Spaceを介して実行する．])

#entry([Process], [Schedulerが実行，Block，再開するユーザ空間の実行単位である．実行文脈とResource参照はProcess Control Blockが保持する．])

#entry([Process Control Block], [Processの実行文脈と状態を保持するCapability Objectである．PCBと略記する．Priority，Quantum，Address Space，Root Capability Node，IPC Buffer，Notification Port，Fault Resolverを管理する．])

#entry([Root Capability Node], [ProcessがCapability DescriptorによるSlot探索を開始するCapability Nodeである．PCB内部のRoot Slotから参照する．])

#entry([Single Kernel Stack], [ProcessごとのKernel Stackを持たず，Coreごとに一つのKernel Stackを共有する実装方式である．A9Nは各Coreへ8 KiBのKernel Stackを割り当てる．])

#entry([SPENCER], [A9N Kernel，A9NLoader-rs，User PayloadをBuildし，Boot可能なDisk Imageを生成するToolKitである．])

#entry([Benno Scheduling], [実行可能なProcessだけをPriorityごとのFIFO Ready Queueで管理するScheduling手法である．A9NのSchedulerは，固定PriorityでQueueを選択し，同じPriorityのProcessをRound-robinで選択する．])

#entry([Virtualization Capability], [Virtual Machine構成用として宣言されたCapability群である．対象RevisionではExperimental Stubであり，Guestを実行する経路は完成していない．])

#entry([Virtual CPU], [Virtual MachineのCPU状態を保持する目的で宣言されたCapability Objectである．対象RevisionではCapability Callを利用できない．])

#entry([Virtual Address Space], [Guest Physical Address Spaceを表す目的でType値が予約されたCapability Objectである．対象Revisionは作成経路とComponent実装を持たない．])

#entry([Virtual Page Table], [Guest用Page Tableを表す目的でType値が予約されたCapability Objectである．対象Revisionは作成経路とComponent実装を持たない．])

== Architecture-dependent Terms

#entry([ABI], [共通Kernel Interfaceを特定のHardware Architectureへ対応させる規約である．Register配置，Context Layout，Page Table，Kernel Call Entryを定める．])

#entry([Hardware Context], [Processの実行再開に必要なCPU Register状態である．Word数，Index，書換え可能なFieldはアーキテクチャごとのABIが定める．])

#entry([Kernel Call Entry], [ユーザ空間からPrivilegeを遷移し，Kernel Call Handlerへ制御を渡すアーキテクチャ依存経路である．])

#entry([x86_64 ABI], [A9Nの共通インターフェースをx86_64 Long Modeへ対応させる規約である．対象Revisionで実装されているHardware Architectureはx86_64だけである．])
