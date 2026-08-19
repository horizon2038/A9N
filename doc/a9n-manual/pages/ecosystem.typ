#import "/components/reference.typ" : term, notice
#import "@preview/cetz:0.5.2"

= Ecosystem

== Scope

#term[A9N Ecosystem]は，A9N Kernelを用いるSystemのBuild，Boot，User-level Runtime，Kernel Call Adapter，共有型を構成するSoftware群である．#term[SPENCER]，#term[Nun]，#term[A9NLoader-rs]，#term[`a9n_abi`]，#term[`a9n_types`]は，異なる実行段階と抽象度を担当する．各Componentの境界を保つことで，Boot処理，ABI，Runtime，Application Policyを個別に更新できる．

SPENCER，Nun，A9NLoader-rs，`a9n_abi`，`a9n_types`の役割はアーキテクチャに依存しない観点から説明する．Kernel Call Register，Entry Stub，UEFI Imageの配置を含むx86_64固有の構成は「x86_64 ABI」に記載する．

== Component Relationship

#figure([
  #set text(size: 8pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(
      stroke: 0.45pt,
      mark: (transform-shape: false, fill: black),
    )

    content((-4.4, 0), [User Payload], name: "payload", frame: "rect", padding: 0.75em, fill: luma(247))
    content((-1.5, 0), [Nun], name: "nun", frame: "rect", padding: 0.75em, fill: luma(247))
    content((1.35, 0), [`a9n_abi`], name: "abi", frame: "rect", padding: 0.75em, fill: luma(247))
    content((4.35, 0), [`a9n_types`], name: "types", frame: "rect", padding: 0.75em, fill: luma(247))

    line("payload.east", "nun.west", mark: (end: ">"))
    line("nun.east", "abi.west", mark: (end: ">"))
    line("abi.east", "types.west", mark: (end: ">"))

    content((-3.0, 0.65), [Runtime], anchor: "south")
    content((-0.08, 0.65), [Kernel Interface], anchor: "south")
    content((2.85, 0.65), [共有型], anchor: "south")

    line(
      "payload.south",
      (-4.4, -1.1),
      (1.35, -1.1),
      "abi.south",
      mark: (end: ">"),
      stroke: (paint: gray, thickness: 0.4pt, dash: "dashed"),
    )
    content((-1.5, -1.4), [独自Runtime], anchor: "north")
  })
], caption: [User-level Componentの依存関係])

User Payloadは，NunをRuntimeとして利用する構成と，`a9n_abi`を直接利用して独自Runtimeを構成する方式を選択できる．両方式は`a9n_types`が定義する共有値を用いる．SPENCERとA9NLoader-rsはCargo Dependencyの層に属さず，BuildとBootを担当する．

== a9n_types

#link("https://crates.io/crates/a9n-types")[`a9n-types`]はCargo Package名，#link("https://github.com/horizon2038/a9n-types")[`a9n_types`]はRustのcrate名である．`a9n_types`は`no_std` crateであり，KernelとUser-level Softwareの境界を越える共有値を定義する．`Word`，`CapabilityDescriptor`，`CapabilityType`，`CapabilityRights`，`CapabilityError`，`MessageInfo`，`InitInfo`，`IpcBuffer`が対象となる．

`a9n_types`は，Kernel Call Number，Capability Operation Number，Register操作，Runtime初期化を実装しない．ABIとして共有する構造体と列挙型には明示的なRepresentationが必要であり，Field順，Alignment，数値をKernel側の定義と一致させる必要がある．

== a9n_abi

#link("https://github.com/horizon2038/a9n-abi")[`a9n_abi`]は，User ModeからA9N Kernelを呼び出す低水準の`no_std` crateである．`a9n_types`へ依存して共有型を再公開し，Kernel Call Number，Capability Operation Number，Message Register Layout，Capability Call Wrapperを提供する．Architecture Moduleは，Trap命令とHardware Registerへの配置を実装する．API Documentationは#link("https://docs.rs/a9n_abi/latest/a9n_abi/")[docs.rsのa9n_abi]で公開されている．

独自Runtimeは，`a9n_abi`を直接利用してEntry Point，IPC Buffer初期化，Panic処理を実装できる．Application PolicyやResource管理は`a9n_abi`の責務に含まれない．Architecture ModuleのRegister配置とKernel Call Entryは，アーキテクチャごとのABIに従う必要がある．

== Nun

#link("https://github.com/horizon2038/Nun")[Nun]は，A9N上でRust製User-level Softwareを構築するための`no_std` Runtimeである．Nunは`a9n_abi`へ依存し，`a9n_abi`が公開する型とKernel Call Wrapperを再公開する．`entry!` MacroはArchitecture Entryを生成し，`InitInfo`をUser Entryへ渡す．初期化処理はInitのIPC BufferをThread-local Storageへ対応付ける．Debug出力とPanic処理もNunが提供する．

Nunは，Executable Loader，Memory Manager，Driver，File System，Process Managerを提供しない．System ServiceとResource Policyは，Nunを利用するUser Payloadが実装する．Entry，IPC Buffer，Debug出力を独自実装する場合，Nunを介さずに`a9n_abi`を利用できる．

== A9NLoader-rs

#link("https://github.com/horizon2038/a9nloader-rs")[A9NLoader-rs]は，A9N Boot Protocolを実装するRust製Bootloaderである．Disk Image内の`kernel.elf`と`init.elf`を読み込み，ELF SegmentとSymbolを解析する．A9NLoader-rsは，FirmwareのMemory MapとHardware情報を`boot_info`へ格納し，Firmware Serviceを終了した後にKernel Entryへ制御を渡す．

A9NLoader-rsはBoot完了後に常駐するSystem Serviceではない．Kernelは`boot_info`からInit Imageを構成し，Initへ`init_info`を渡す．Boot情報の共通構造は「Boot and Init Protocol」に，特定ArchitectureでのFirmwareとEntry規約は「x86_64 ABI」に記載する．

== SPENCER

#link("https://github.com/horizon2038/spencer")[SPENCER]は，A9N-based SystemのSource取得，Build，Disk Image生成，実行を統合するToolKitである．SPENCERのGit Submoduleは，A9N，Nun，A9NLoader-rsを保持する．`a9n_abi`と`a9n_types`はGit Submoduleではなく，NunまたはUser PayloadのCargo Dependencyとして解決される．

SPENCERはA9N Kernel，A9NLoader-rs，User Payloadを選択した構成でBuildし，三つのExecutableをBoot可能なDisk Imageへ格納する．Command，既定値，Artifact Path，起動確認は「Getting Started」を正本とする．SPENCERが指定するSubmodule構成と，User Payloadの`Cargo.lock`が確定するCargo Dependencyを一つの組合せとして扱う必要がある．

== Layer Selection

共有構造体と列挙型だけを扱うLibraryは`a9n_types`を用いる．Kernel Call Adapterを独自に構成するRuntimeは`a9n_abi`を用いる．標準のRust EntryとIPC Buffer初期化を必要とするUser PayloadはNunを用いる．Kernel，Bootloader，User Payloadを一括してBuild・実行する開発環境はSPENCERを用いる．独自Bootloaderは，A9NLoader-rsと同じA9N Boot Protocolを実装する必要がある．

#notice([CAUTION], [
  `a9n_types`の共有値，`a9n_abi`のOperation Number，A9N KernelのHeader，A9NLoader-rsのBoot構造体は，同じ境界仕様を表す．一つのComponentだけを更新すると，Compile成功後のKernel Call失敗，Init情報の誤読，Boot Failureが発生し得る．SPENCERのSubmoduleとPayloadの`Cargo.lock`が固定する組合せを基準にBuildと起動を検証する必要がある．
])
