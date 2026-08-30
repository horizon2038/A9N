#import "/components/reference.typ" : reference_table, fields, notice, term

= x86_64 ABI

== Scope

#term[x86_64 ABI]は，A9Nの共通インターフェースをx86_64 Long Modeへ対応させる規約である．本章では，Register，Page Table，I/O PortのHAL実装，Boot情報，割り込み，VMXの順にx86_64固有の規約を示す．AArch64固有の規約は「AArch64 ABI」に記載する．

#reference_table(
  (1.5fr, 2.8fr),
  ([項目], [値]),
  [Word幅], [64 bit．],
  [基本Page Size], [4 KiB．`PAGE_SIZE = 4096`，`INITIAL_FRAME_SIZE_BITS = 12`．],
  [User Address], [`0 <= address < 0x0000_7fff_ffff_ffff`．上限値は含まれない．],
  [Kernel Direct Map], [物理Addressへ`KERNEL_VIRTUAL_BASE`を加えたVirtual Addressを用いる．],
  [IRQ数], [`IRQ_NUMBER_MAX = 256`．IRQ番号は0から255である．],
  [実行Model], [SMPをBuild時に選択できる．最大64 Core．共有Address SpaceのRemote TLB Shootdownを実装する．],
)

== Kernel Call Register

Kernel Callには`syscall`命令を用いる．`RAX`にCall Numberを置き，Virtual Message Registerの`MR0`から`MR9`をGeneral-purpose Registerへ割り当てる．`MR10`以降は，実行中Processの`ipc_buffer.messages[index]`へ割り当てる．`MESSAGE_BUFFER_SIZE_MAX`は494であり，`messages`の有効Indexは0から493である．x86_64 HALはMR Indexの範囲を検査しない．ユーザ空間は`MRi`を使用する前に$i < 494$を満たすことを検証する必要がある．

#block(breakable: false)[
  #reference_table(
    (1fr, 2.2fr),
    ([ABI上の値], [Register]),
    [Kernel Call], [`RAX`],
    [`MR0`], [`RDI`],
    [`MR1`], [`RSI`],
    [`MR2`], [`RDX`],
    [`MR3`], [`R8`],
    [`MR4`], [`R9`],
    [`MR5`], [`R10`],
    [`MR6`], [`R12`],
    [`MR7`], [`R13`],
    [`MR8`], [`R14`],
    [`MR9`], [`R15`],
    [`MR10..`], [`ipc_buffer.messages[index]`],
  )
]

`syscall`命令はUser RIPを`RCX`へ，User RFLAGSを`R11`へ保存する．Kernel Call Entryは保存値を`sysret`に用いる．ユーザ空間は`RCX`と`R11`が呼出し後も保存されると仮定してはならない．対象Revisionにはユーザ空間向けのC Function Signatureがない．Wrapperは，C ABIの引数順ではなく，Message Register番号に従って値を置く必要がある．

== Hardware Context

PCBの`READ_REGISTER`と`WRITE_REGISTER`が扱うHardware Contextは23 Wordである．Fault IPCの`INVALID_KERNEL_CALL`も，同じ並びを`MR7`以降に格納する．

#block(breakable: false)[
  #reference_table(
    (0.8fr, 2fr),
    ([Index], [Register]),
    [`0`], [`RAX`],
    [`1`], [`RBX`],
    [`2`], [`RCX`],
    [`3`], [`RDX`],
    [`4`], [`RDI`],
    [`5`], [`RSI`],
    [`6`], [`RBP`],
    [`7`], [`R8`],
    [`8`], [`R9`],
    [`9`], [`R10`],
    [`10`], [`R11`],
    [`11`], [`R12`],
    [`12`], [`R13`],
    [`13`], [`R14`],
    [`14`], [`R15`],
    [`15`], [`RIP`],
    [`16`], [`CS`],
    [`17`], [`RFLAGS`],
    [`18`], [`RSP`],
    [`19`], [`SS`],
    [`20`], [`GS_BASE`],
    [`21`], [`FS_BASE`],
    [`22`], [`ENTER_FROM`],
  )
]

PCBの`thread_local_base`は`GS_BASE`へ保存する．`WRITE_REGISTER`はUser Address，Segment Selector，RFLAGS，`ENTER_FROM`を検証しない．Process Managerは，Privilege Levelを変えないFieldだけを書き換える必要がある．

#notice(
  [CAUTION],
  [実行中Processが自身のPCBへ`READ_REGISTER`を実行する場合，`register_count`は8未満にする必要がある．`register_count >= 8`では，Index 0の返却先`MR3`がHardware ContextのIndex 7（`R8`）を上書きしてからIndex 7を読むため，Index 7以降は呼出し開始時点のSnapshotにならない．23 WordのContextを確認するProcess Managerは，対象Processを停止して別Processから`READ_REGISTER`を実行する必要がある．],
)

== Page Table

x86_64のAddress Spaceは，4階層のPage Tableで構成される．Address Spaceを作成すると，HALはKernel Root Page Tableを新しいPML4へCopyする．ユーザ空間のMappingにはLower Halfだけを使用する．PML4[511]は通常のMappingに使用せず，Address Spaceを実行中のCoreを表す64 bit Owner BitmapとしてHALが予約する．

#reference_table(
  (1fr, 0.8fr, 1fr, 2.3fr),
  ([Level], [Depth], [範囲], [役割]),
  [`PML4`], [`4`], [512 GiB], [Address SpaceのRoot．],
  [`PDPT`], [`3`], [1 GiB], [中間Table，または1 GiB FrameのLeaf．],
  [`PD`], [`2`], [2 MiB], [中間Table，または2 MiB FrameのLeaf．],
  [`PT`], [`1`], [4 KiB], [4 KiB FrameのLeafを格納する．],
)

Frame Size Bitsは12，21，30である．12は常に利用できる．21と30は，Boot時に検出したCPU機能に依存する．`CAN_MAP_FRAME_SIZE_BITS`は，指定したSizeを利用できる場合に成功を返す．Page TableのDepthは1から3である．

#reference_table(
  (1fr, 0.8fr, 2.7fr),
  ([Attribute], [値], [Page Entry]),
  [`READ`], [`0x1`], [PresentなUser Mappingを作る．独立したRead-disable Bitがないため，実装はBit 0を個別に判定しない．],
  [`WRITE`], [`0x2`], [`rw`を1にする．],
  [`EXECUTE`], [`0x4`], [`execute_disable`を0にする．未指定の場合はNXを設定する．],
)

4 KiB FrameはPTへ，2 MiB FrameはPDへ，1 GiB FrameはPDPTへMapする．FrameのPhysical Addressは12 bit右ShiftしてPage Entryへ格納する．Virtual AddressとPhysical AddressのAlignmentはAddress Space Callで検査されないため，ユーザ空間がFrame Sizeに合わせる必要がある．

Owner BitmapのBit $n$はCore $n$を表す．Context SwitchでAddress Spaceが変わる場合，HALは次のPML4へ現在CoreのBitをSetし，`CR3`を更新してから，保存した以前の`CR3`が指すPML4から同じBitをClearする．同じAddress Space間では`CR3`とBitmapの更新を省略する．

Mapping変更時，Bitmapが0なら即時のTLB無効化を行わない．現在CoreのBitがあれば，Frame操作では対象Addressへ`invlpg`を実行し，Page Table操作では`CR3`を再Loadして配下を含むTLBをFlushする．別CoreのBitがあれば`IPI_INVALIDATE_TLB`を送る．IPI Handlerは`CR3`を再LoadしてTLBをFlushする．PCIDは使用しない．

== I/O Port HAL

I/O Port CapabilityのObject ModelとCapability Callは「I/O Port」に記載する．x86_64 HALは，I/O Addressを16 bit Port番号として`in`命令と`out`命令へ渡す．Word幅のAddressは`uint16_t`へ変換されるため，上位bitは切り捨てられる．

Byte Width 1，2，4は，順に8 bit，16 bit，32 bitの命令へ対応する．別のWidthは`hal_error::ILLEGAL_ARGUMENT`となり，Capability Callでは`FATAL`へ変換される．Write Dataは命令幅へ切り詰められ，Read Dataの上位bitは0となる．

== Boot and Init

`INITIAL_FRAME_SIZE_BITS`は12であり，Init Imageを4 KiB単位でMapする．`init_image_size`にはByte数ではなく4 KiB Frame数を設定する．Kernelは新しいPML4を作成し，Init ImageをVirtual Address 0から連続してMapする．IPC Bufferも4 KiB境界へ配置する必要がある．Kernel初期AllocatorのSizeは`PAGE_SIZE * 1024`，すなわち4 MiBである．

`boot_info.arch_info[0]`は，ACPI RSDPのPhysical Addressである．HALはKernel Direct Map Addressへ変換してACPIの初期化に渡す．0は受理されない．`arch_info[1..127]`は使用しない．

InitのStack Pointerは0のままUser Modeへ復帰する．Init Entryは，CまたはC++のPrologueを実行する前に，Assembly StubでMap済みStackの上端を`RSP`へ設定する必要がある．

== Interrupt

IRQ番号は0から255である．Hardware IRQをNotification Portへ配送した後，KernelはIRQをAcknowledgeしてMaskする．DriverはDevice固有のStatusを処理してからInterrupt Portの`ACK`を呼び，IRQをUnmaskする必要がある．

Bind済みHandlerがないIRQはユーザ空間へ配送されない．Bind済みHandlerがない場合，カーネルはIRQをAcknowledgeまたはMaskしない．Interrupt Controllerに残る状態は，x86_64 HALのController実装に依存する．

/*
== VMX

x86_64 HALには，VMCS Region，VMX MSR，VMCS Field，VMX Resultを扱う補助実装がある．`try_init_virtual_cpu()`はVMCS Revision IDを読み，Regionを初期化する．GenericによるVirtual CPUの作成は`try_init_virtual_cpu()`を呼ばない．

`enter_virtual_machine()`と`inject_virtual_irq()`は，処理を行わずに成功を返す．Guest Entry，Guest Exit，Exit Reason，Guest Register，EPT，Virtual Interrupt，CPU Pinning，Revokeの規約は実装されていない．Virtualization Capabilityは利用できないものとして扱う必要がある．

#notice(
  [WARNING],
  [Hardware Contextの書換え，Page Tableの再利用，I/O Portの配布はPrivilege境界に直結する．対象Revisionには未実装の検査と空のRevoke処理が残る．ユーザ空間のResource Managerは，値の範囲，破棄順序，Capabilityの配布先をKernel Callの前に検証できる．事前検査の有無にかかわらず，Kernel Callの成否は返却されたResultで判定する．],
)
*/

== Initial Capability Descriptors

Init Root Nodeの`radix_bits`は8，`ignore_bits`は0である．Root Slot $n$を直接指定するDescriptorは，Depth 16，Encoded Depth 8であり，次式となる．

$
  d_"root"(n) = "0x0800_0000_0000_0000" + n times 2^48.
$

PCBを保持するRoot Slot 1は`0x0801_0000_0000_0000`，Generic Nodeを保持するRoot Slot 7は`0x0807_0000_0000_0000`である．Root Slot 7のGeneric Nodeは`radix_bits = 7`である．Generic Node内のSlot $g$を指定するDescriptorは，Depth 23，Encoded Depth 15であり，次式となる．

$
  d_"generic"(g) = "0x0f07_0000_0000_0000" + g times 2^41,
  quad 0 <= g < 128.
$

`generic_list_count`は，KernelがGeneric Nodeへ設定したSlot数の確認に用いる．対象Revisionには「Generic Descriptor Generation」に記載したCountの不整合があるため，Descriptorを構成する前にGeneric Nodeの有効範囲とBootloaderのMemory Map制約を照合する必要がある．


== ABI Conformance Example

Repositoryには，x86_64 ABI境界を検査する自己完結型Payloadとして`doc/a9n-manual/examples/x86_64-hello`を収録する．検査対象は，Init Entry，Stack，`init_info` Layout，Register-backed MR，IPC Buffer-backed MR，`CAPABILITY_CALL`，Capability Error，`DEBUG`，`YIELD`である．

ABI Conformance Exampleは，Nunと`a9n_abi`を使用する標準User Payloadとは目的が異なる．ABI Conformance ExampleをApplicationまたはSystem Initの雛形として使用してはならない．Build，ELF Layout検査，実行，期待するSerial出力は`doc/a9n-manual/examples/x86_64-hello/README.md`に記載する．本章は，Exampleが検査するABI値と不変条件だけを定義する．
