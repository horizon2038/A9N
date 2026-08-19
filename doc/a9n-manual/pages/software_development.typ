#import "/components/reference.typ" : reference_table, notice, term
#import "@preview/cetz:0.5.2"

= User-level System Architecture

== Scope

#term[User-level Software]は，A9NのUser Modeで動作し，Kernel CallによってKernel Objectを操作するSoftwareである．A9N Kernelは，Executable Loader，File System，POSIX Process APIを提供しない．Boot時に最初のUser-level Softwareとして#term[Init Image]を一つ起動し，後続のProcessとSystem Serviceを構成する責務をInitへ委譲する．

本章は，アーキテクチャに依存しないUser-level Systemの構成要素，責務，Authority，Resource Lifetimeを説明する．InitとServiceを実装する手順は「Building Init and Services」に，Kernel Call EntryとRegister配置は「x86_64 ABI」に記載する．


== Development Model

Init Imageは，ApplicationのEntry Pointだけでは成立しない．Language Runtimeへ入る前の初期化，`init_info`の取得，Kernel Callへの変換，Capability管理，Boot可能なImageへのLinkが必要である．各層の責務は次の通りである．

#reference_table(
  (1.3fr, 3.2fr),
  ([層], [説明]),
  [Application], [Resource Policy，Driver，Memory Manager，Protocol処理を実装する．],
  [Runtime], [Stack，静的領域，Panic処理，必要なLanguage機能を初期化する．],
  [#term[Kernel Call Adapter]], [Operation固有の値をVirtual Message Registerへ配置し，Kernel Call Entryを呼び出し，結果を型付きの値へ戻す．],
  [Image], [Entry Point，`init_info`領域，IPC Buffer，Stack，Load可能なProgram領域を保持する．],
  [Boot], [Kernel ImageとInit Imageを配置し，`boot_info`を構成してKernelへ制御を渡す．],
)

#figure([
  #set text(size: 8pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(
      stroke: 0.45pt,
      mark: (transform-shape: false, fill: black),
    )

    content((-3, 0), [Application], name: "application", frame: "rect", padding: 0.65em, fill: luma(247))
    content((0, 0), [Kernel Call Adapter], name: "adapter", frame: "rect", padding: 0.65em, fill: luma(247))
    content((3, 0), [Init Image], name: "image", frame: "rect", padding: 0.65em, fill: luma(247))
    content((3, -1.8), [Boot Protocol], name: "boot", frame: "rect", padding: 0.65em, fill: luma(247))
    content((0, -1.8), [A9N Kernel], name: "kernel", frame: "rect", padding: 0.65em, fill: luma(235))
    content((-3, -1.8), [Service Process], name: "service", frame: "rect", padding: 0.65em, fill: luma(247))

    line("application.east", "adapter.west", mark: (end: ">"))
    line("adapter.east", "image.west", mark: (end: ">"))
    line("image.south", "boot.north", mark: (end: ">"))
    line("boot.west", "kernel.east", mark: (end: ">"))
    line("kernel.west", "service.east", mark: (end: ">"))
    line("service.north", (-3, -0.9), (-1, -0.9), "adapter.south", mark: (end: ">"))
  })
], caption: [SourceからService Processまでの開発境界])

Kernel Call Adapterは，Architecture ABIだけを局所化する．ApplicationはHardware Registerを直接扱わず，Capability Descriptor，Operation，Message Fieldを指定する．新しいArchitectureへ移植する場合は，Kernel Call EntryとVirtual Message Registerの保存先を差し替え，Object Operationの呼出し側を共有する．

== Implementation Units

User-level SoftwareのSourceは，Kernel InterfaceとApplication Policyを分離する．最小構成は次のUnitから成る．

#reference_table(
  (1.2fr, 1.5fr, 2.2fr),
  ([*Unit*], [*入力と出力*], [*責務*]),
  [Entry], [`init_info` PointerからRuntime Entryへ移る．], [Stack，静的領域，IPC Buffer，Language Runtimeを初期化する．],
  [Kernel Call Adapter], [型付き引数からMR列を作り，Resultを返す．], [`CAPABILITY_CALL`と`YIELD`のArchitecture Entryを局所化する．],
  [Capability Inventory], [Descriptor，Type，Rights，Ownerを記録する．], [Capability Spaceの割当て，委譲，失効を追跡する．],
  [Object Builder], [GenericとDestination SlotからObject群を作る．], [Address Space，PCB，IPC Port等の作成順とRollbackを管理する．],
  [Protocol], [Message LayoutとService Errorを定義する．], [IPC Request，Reply，Notification，Capability Transferを符号化する．],
  [Policy], [Kernel ObjectとProtocolを利用する．], [Memory Manager，Driver，Process Manager，File System等を実装する．],
)

EntryとKernel Call AdapterだけがArchitecture固有コードを含む．Capability Inventory，Object Builder，Protocol，Policyは，Architecture固有のAddress WidthやPage AttributeをParameterとして受け取り，共通のObject Operationを利用する．

== Software Roles

User-level Systemは，Init，System Service，Clientの3種類に分けて設計できる．A9N KernelはRoleを識別しない．各Processが保持するCapabilityと，IPC ProtocolがRoleを決める．

#reference_table(
  (1.1fr, 1.5fr, 2.4fr),
  ([*Role*], [*保持するResource*], [*責務*]),
  [Init], [Initial Capability Space，Generic，Interrupt Region，I/O Port], [KernelからResourceを受け取り，Objectを作成し，CapabilityをServiceへ配布する．],
  [System Service], [Service用PCB，Address Space，Root Node，IPC Port], [Memory，Driver，Process，File System等のPolicyを実装し，IPCで機能を提供する．],
  [Client], [Service IPC PortとClient自身のMemory Resource], [Requestを構成し，ReplyまたはNotificationを処理する．],
)

Initは，全Resourceを自身で使い続けるProcessではない．System構成に必要なCapabilityを作成し，`MINT`でRightsを制限してServiceへ配布する．ClientにはServiceのIPC Portを渡し，Device Generic，Interrupt Region，PCB等の管理Capabilityを直接渡さない構成を基本とする．

== Resource Lifetime and Concurrency

Capability ObjectとWait Queueには，User-level System全体を覆うTransactionや共通Lockが存在しない．同じNode，Generic，IPC Port，Notification Port，PCBを複数Processまたは複数Coreから操作する場合，Resource ManagerはObject単位で直列化する．Operationの完了後に台帳を更新し，台帳更新前のObjectを別のWorkerへ渡さない．

Serviceを停止する際のResource依存関係は，受付停止，Waiterの解消，Process停止，Mapping解除，Capability失効，Generic Revokeの順に解消する．具体的な終了手順は「Building Init and Services」に記載する．


IPC PortのRevokeはWaiterを自動的に起こさず，GenericのRevokeは派生Object MemoryをClearしない．破棄順序はKernel Callの成功だけでなく，User-level台帳と各ProcessのStateを用いて確認する．

