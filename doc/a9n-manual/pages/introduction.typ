#import "/components/reference.typ" : reference_table, term
#import "@preview/cetz:0.5.2"

= Introduction

== Purpose and Audience

#term[A9N Microkernel]@A9N:online は，第3世代のCapability-based Microkernelである．A9Nは，Capabilityに基づく権限管理，Processの実行，仮想メモリ，IPC，Notification，割り込み配送の機構をKernelへ置き，Resourceの配布とSystem ServiceのPolicyをUser Modeへ分離する．

本書は，A9Nを初めて扱うOS開発者が，標準構成の起動，共通Kernel Interfaceの理解，Kernel Objectの操作，InitとServiceの実装，Architecture固有ABIの確認，HALの移植を行える状態を目標とする．A9NのSource Repositoryは，GitHubの#link("https://github.com/horizon2038/A9N")[horizon2038/A9N]で公開されている．

== System Boundary

A9Nは，Kernel，#term[HAL]（Hardware Abstraction Layer），#term[User-level System]の3領域に分かれる．KernelはArchitectureに依存しないObject Modelと実行機構を実装する．HALはCPU，Memory，Interrupt，Timer，I/OをKernel Interfaceへ接続する．User-level Systemは#term[Init]を起点としてCapabilityを配布し，Memory Manager，Driver，File System，System ServiceのPolicyを実装する．

#reference_table(
  (1.1fr, 1.8fr, 2.7fr),
  ([層], [主なInterface], [責務]),
  [User-level System], [Capability Call，IPC，Notification], [Init，Resource Manager，Driver，Service，Applicationを構成する．],
  [Kernel], [Kernel Object，Scheduler], [CapabilityのAuthority，Process，Address Space，IPC，Notification，Faultを管理する．],
  [HAL], [HAL Interface], [Architecture固有のContext，Memory Mapping，Interrupt，Timer，I/O，Kernel Call Entryを実装する．],
)

#figure([
  #set text(size: 8pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(
      stroke: 0.45pt,
      mark: (transform-shape: false, fill: black),
    )

    content((0, 0), [User-level System], name: "user", frame: "rect", padding: 0.75em, fill: luma(247))
    content((0, -1.8), [A9N Kernel], name: "kernel", frame: "rect", padding: 0.75em, fill: luma(235))
    content((0, -3.6), [HAL], name: "hal", frame: "rect", padding: 0.75em, fill: luma(247))
    content((0, -5.4), [Hardware], name: "hardware", frame: "rect", padding: 0.75em, fill: luma(247))

    line("user.south", "kernel.north", mark: (end: ">"))
    line("kernel.south", "hal.north", mark: (end: ">"))
    line("hal.south", "hardware.north", mark: (end: ">"))

    content((0.35, -0.9), [Capability Call / `YIELD`], anchor: "west")
    content((0.35, -2.7), [HAL Interface], anchor: "west")
    content((0.35, -4.5), [Architecture-specific Control], anchor: "west")
  })
], caption: [A9NのSystem Boundary])

A9Nが通常のUser-level Softwareへ提供するKernel Callは，#term[Capability Call]と`YIELD`の2種類である．Capability Callは，#term[Capability Descriptor]で指定したCapabilityを介してKernel ObjectのOperationを実行する．`YIELD`は実行権をSchedulerへ返す．個別のKernel Object Operationを独立したSystem Callとして公開しない．

== Policy/Mechanism Separation

#term[機構と方針の分離]は，Resourceの操作と保護に必要なMechanismをKernelへ置き，Resourceの配分やServiceの構成を決めるPolicyをUser-level Softwareへ委ねる設計原則である．Hydraは，Scheduling，Paging，ProtectionのMechanismをKernelへ置き，外部のPolicyがMechanismを操作する構成としてこの原則を示した@LevinEtAl:1975．

A9Nでは，CapabilityによるAuthorityの検査，Kernel ObjectのOperation，Processの実行，Memory Mapping，IPC，Notification，割り込み配送をMechanismとして提供する．InitとUser-level Systemは，CapabilityをどのProcessへ配布するか，MemoryとCPU時間をどのServiceへ割り当てるか，DriverとSystem Serviceをどのように構成するかをPolicyとして決定する．Kernelに残るType検査，Rights検査，Scheduling規則，Resource競合の処理は，保護境界と実行可能性を維持するための共通規則である．

== Manual Structure

Part Iは，標準構成を起動してUser Payloadの実行を確認する．Part IIは，CapabilityとKernel Callの共通規約を定義する．Part IIIは，Capability Node，Generic，Memory，Process，IPC，Notification，Interrupt，I/OをKernel Objectごとに記載する．Part IVは，Boot後のInitとUser-level Systemの構成を説明する．Part Vは，x86_64固有ABIとHAL移植手順を扱う．Part VIは，A9Nと組み合わせるSoftware Componentを説明する．

最初にA9Nを起動する読者は「Getting Started」へ進む．Kernel Interfaceを実装する読者はPart IIからPart IVまでを順に読む．既存Runtimeを使わずにEntryまたはKernel Call Adapterを実装する読者は，共通Kernel Interfaceを確認した後に「x86_64 ABI」を読む．新しいArchitectureを実装する読者は，既存の具体例として「x86_64 ABI」を確認してから「HAL Porting Guide」へ進む．
