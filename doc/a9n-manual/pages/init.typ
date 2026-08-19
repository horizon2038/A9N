#import "/components/reference.typ" : reference_table, notice, term
#import "@preview/fletcher:0.5.8" as fletcher: diagram, node, edge

= Boot and Init Protocol

== Scope

#term[Init]は，カーネルが起動する最初のUser Processである．BootloaderはKernel Image，#term[Init Image]，Memory Map，Architecture情報を準備する．カーネルは`boot_info`を読み，InitのCapability Space，Address Space，Hardware Contextを構成する．

対象RevisionのKernelは，Init ImageのELF Headerを解析しない．BootloaderがImage Formatを解析し，物理配置とEntry情報を`boot_info`へ設定する必要がある．Kernelは，`boot_info`のField間に矛盾がないかを検査しない．

== Boot Flow

BootからInit実行までの制御順序は次の通りである．

+ BootloaderがKernel ImageとInit Imageを物理メモリへ配置する．
+ Bootloaderが`boot_info`を構成し，Pointerを`kernel_entry()`へ渡す．
+ KernelがCPU-local DataとHALを初期化する．
+ KernelがInterrupt ManagerとProcess Managerを初期化する．
+ Kernelが`create_init()`でInit Processを作成する．
+ SchedulerがInitを選択し，HALがUser ContextへRestoreする．

#figure([
  #diagram(
    node-stroke: 0.08em,
    node-fill: luma(246),
    node-inset: 0.75em,
    spacing: 2em,

    node((0, 0), [Bootloader \ Image + `boot_info`], name: <bootloader>),
    node((2, 0), [`kernel_entry()`], name: <kernel-entry>),
    node((4, 0), [HAL + Kernel Manager \ Initialization], name: <manager-init>),
    node((4, 1.3), [`create_init()`], name: <create-init>),
    node((2, 1.3), [Scheduler \ Benno Scheduling \ Ready Queue], name: <ready-queue>),
    node((0, 1.3), [Init User Context], name: <init-context>),

    edge(<bootloader>, <kernel-entry>, "-|>", label-side: center),
    edge(<kernel-entry>, <manager-init>, "-|>", label-side: center),
    edge(<manager-init>, <create-init>, "-|>", label-side: center),
    edge(<create-init>, <ready-queue>, [`READY`], "-|>", label-side: center),
    edge(<ready-queue>, <init-context>, [`restore`], "-|>", label-side: center),
  )
], caption: [BootloaderからInit User Contextまでの制御順序])

`create_init()`は，Create PhaseとConfigure Phaseに分かれる．Create Phaseでは，PCB，Root Node，Page Table Node，Frame Node，Generic NodeをKernel初期Allocatorから作成する．Configure Phaseでは，`init_info`，I/O Port，Address Space，Frame，Generic，Interrupt Region，Scheduling Stateを設定する．

Kernel初期Allocatorは`PAGE_SIZE * 1024` ByteのLinear Allocatorであり，割り当てた領域を解放しない．Init用のNode配列とPage Tableも同じ領域から割り当てる．具体的なByte数はアーキテクチャごとの基本Page Sizeによって決まる．

== Word単位のLayout

Layout表は，Structure先頭からのOffsetとMember SizeをWord単位で表す．32-bit Wordは4 Byte，64-bit Wordは8 Byteである．1 Wordより小さいMemberとPaddingに限り，Word先頭からのByte位置とByte数を併記する．OffsetとSizeの各値には，`Word`，`Byte`，またはByteの単位記号`B`を必ず付記する．したがって，表を読むためにWord幅からOffsetを計算する必要はない．

以下のLayoutは，Pointerが1 Word，`memory_map_type`が4 Byte，`bool`が1 ByteであるC++ ABIを対象とする．新しいArchitectureでは，これらのSizeとAlignmentをBuild時に検証する必要がある．

== boot_info

`boot_info`は`__attribute__((packed))`で宣言される．`memory_info`のSizeを3 Words，`init_image_info`のSizeを5 Wordsとして，Top-level Layoutは次の順序となる．

#reference_table(
  (1.7fr, 1.15fr, 1.15fr, 1.15fr, 1.15fr),
  ([Field], [Offset (32-bit)], [Size (32-bit)], [Offset (64-bit)], [Size (64-bit)]),
  [`boot_memory_info`], [Word 0], [3 Words], [Word 0], [3 Words],
  [`boot_init_image_info`], [Word 3], [5 Words], [Word 3], [5 Words],
  [`arch_info[128]`], [Word 8], [128 Words], [Word 8], [128 Words],
)

`boot_memory_info`はMemory Size，Memory Map Entry数，Memory Map Pointerを保持する．`boot_init_image_info`はInit Imageの物理配置とUser Address情報を保持する．`arch_info`はArchitecture固有のBoot情報を保持する．

`boot_info`全体のSizeは136 Wordsである．`packed`指定はStructure自体のAlignmentを1 Byteまで下げるため，Bootloaderは`boot_info`先頭をWord境界へ配置する必要がある．Kernelは`boot_info` PointerのAlignmentを検査しない．

=== memory_info

#block(breakable: false)[
  `boot_memory_info`の型である`memory_info`は，PointerをWord境界へ配置するPaddingを持つ．

  #reference_table(
    (1.7fr, 1.15fr, 1.15fr, 1.15fr, 1.15fr),
    ([Field], [Offset (32-bit)], [Size (32-bit)], [Offset (64-bit)], [Size (64-bit)]),
    [`memory_size`], [Word 0], [1 Word], [Word 0], [1 Word],
    [`memory_map_count`], [Word 1], [2 Byte], [Word 1], [2 Byte],
    [Padding], [1 Word+2 B], [2 Byte], [1 Word+2 B], [6 Byte],
    [`memory_map`], [Word 2], [1 Word], [Word 2], [1 Word],
  )
]

`memory_size`はKernelへ通知するMemory Size，`memory_map_count`はMemory Map Entry数である．`memory_map`は`memory_map_entry` Array先頭を指す．

どちらのWord幅でも，`memory_info`全体のSizeは3 Wordsである．`memory_map`が指す各`memory_map_entry`もWord境界へ配置する．

=== memory_map_entry

#reference_table(
  (1.7fr, 1.15fr, 1.15fr, 1.15fr, 1.15fr),
  ([Field], [Offset (32-bit)], [Size (32-bit)], [Offset (64-bit)], [Size (64-bit)]),
  [`start_physical_address`], [Word 0], [1 Word], [Word 0], [1 Word],
  [`page_count`], [Word 1], [1 Word], [Word 1], [1 Word],
  [`type`], [Word 2], [1 Word], [Word 2], [4 Byte],
  [Padding], [なし], [なし], [2 Words+4 B], [4 Byte],
)

`start_physical_address`はEntry先頭のPhysical Address，`page_count`はEntryが含む基本Page数である．`type`は`FREE`，`DEVICE`，`RESERVED`を表す．

どちらのWord幅でも，`memory_map_entry`全体のSizeは3 Wordsである．`memory_map_type`は明示的なUnderlying Typeを持たないため，新しいToolchainでは4 Byteであることと数値表現を確認する必要がある．

=== init_image_info

`boot_init_image_info`の型である`init_image_info`は，5個のWord-sized Memberから成る．

#reference_table(
  (1.7fr, 1.15fr, 1.15fr, 1.15fr, 1.15fr),
  ([Field], [Offset (32-bit)], [Size (32-bit)], [Offset (64-bit)], [Size (64-bit)]),
  [`loaded_address`], [Word 0], [1 Word], [Word 0], [1 Word],
  [`init_image_size`], [Word 1], [1 Word], [Word 1], [1 Word],
  [`entry_point_address`], [Word 2], [1 Word], [Word 2], [1 Word],
  [`init_info_address`], [Word 3], [1 Word], [Word 3], [1 Word],
  [`init_ipc_buffer_address`], [Word 4], [1 Word], [Word 4], [1 Word],
)

`loaded_address`はInit Image先頭のPhysical Address，`init_image_size`は割り当てた初期Frame数，`entry_point_address`はInitのUser Entry Pointである．`init_info_address`と`init_ipc_buffer_address`は，それぞれImage先頭から対象領域までのOffsetである．

`loaded_address`は初期Frame Sizeに合わせて配置する．初期Frame Sizeを$2^"INITIAL_FRAME_SIZE_BITS"$ Byteとする．KernelはInit ImageのFrame $i$を，Physical Address `loaded_address + 2^INITIAL_FRAME_SIZE_BITS * i`からVirtual Address $2^"INITIAL_FRAME_SIZE_BITS" times i$へMapする．Init ImageのUser Virtual Baseは0である．`entry_point_address`，`init_info_address`，`init_ipc_buffer_address`は0起点Mappingと一致する必要がある．Kernelは`loaded_address + init_info_address`へ起動情報を書き込み，`init_ipc_buffer_address`をInit側のVirtual Addressとしても使用する．

#notice(
  [WARNING],
  [`init_image_size`は名称に反してFrame数である．Byte数を設定すると，Kernelは過大なFrame数をMapし，意図しない物理メモリをInitへ公開する．Bootloaderは`ceil(image_bytes / initial_frame_size)`を設定し，配置済み範囲と一致させる必要がある．],
)

== init_info

Kernelは，Init Image内の予約領域へ`init_info`を書き込む．`init_info`全体は一つの基本Pageへ収まる．Init Entryの引数Registerに`init_info*`を渡す規約はない．Initは，Link時に確定したUser Virtual Addressから`init_info`を取得する．

=== generic_descriptor

`generic_descriptor`は2 Wordsである．`size_radix`と`is_device`はWord 1の先頭に並び，残りはPaddingとなる．

#reference_table(
  (1.7fr, 1.15fr, 1.15fr, 1.15fr, 1.15fr),
  ([Field], [Offset (32-bit)], [Size (32-bit)], [Offset (64-bit)], [Size (64-bit)]),
  [`address`], [Word 0], [1 Word], [Word 0], [1 Word],
  [`size_radix`], [Word 1], [1 Byte], [Word 1], [1 Byte],
  [`is_device`], [1 Word+1 B], [1 Byte], [1 Word+1 B], [1 Byte],
  [Padding], [1 Word+2 B], [2 Byte], [1 Word+2 B], [6 Byte],
)

=== init_info Layout

#reference_table(
  (1.7fr, 1.15fr, 1.15fr, 1.15fr, 1.15fr),
  ([Field], [Offset (32-bit)], [Size (32-bit)], [Offset (64-bit)], [Size (64-bit)]),
  [`kernel_major_version`], [Word 0], [1 Word], [Word 0], [1 Word],
  [`kernel_minor_version`], [Word 1], [1 Word], [Word 1], [1 Word],
  [`kernel_patch_version`], [Word 2], [1 Word], [Word 2], [1 Word],
  [`kernel_pre_release[32]`], [Word 3], [8 Words], [Word 3], [4 Words],
  [`kernel_build_meta_data[32]`], [Word 11], [8 Words], [Word 7], [4 Words],
  [`arch_info[128]`], [Word 19], [128 Words], [Word 11], [128 Words],
  [`ipc_buffer`], [Word 147], [1 Word], [Word 139], [1 Word],
  [`generic_list[128]`], [Word 148], [256 Words], [Word 140], [256 Words],
  [`generic_list_count`], [Word 404], [1 Word], [Word 396], [1 Word],
)

Version FieldはKernel Versionを，`kernel_pre_release`と`kernel_build_meta_data`はVersion文字列を保持する．`arch_info`は`boot_info.arch_info`のCopyである．`ipc_buffer`は`init_ipc_buffer_address`と同じUser Virtual Addressを保持する．`generic_list`はMemory Mapから生成したDescriptorであり，`generic_list_count`は有効なElement数を表す．

`init_info`全体のSizeは，32-bit Wordでは405 Words，64-bit Wordでは397 Wordsである．

Generic Descriptorが表す範囲は$["address", "address" + 2^"size_radix")$である．`generic_list_count`は，有効な`generic_list` Element数を表す．実装上のCount制約は「Generic Descriptor Generation」に記載する．

== Initial Capability Space

Init Root Nodeは256 Slotを持つ．Kernelが構成するRoot Slotは次の通りである．

#reference_table(
  (0.6fr, 1.7fr, 2.7fr),
  ([*Slot*], [*Name*], [*Initial Capability*]),
  [`0`], [`RESERVED`], [`NONE`．],
  [`1`], [`PROCESS_CONTROL_BLOCK`], [Init自身のPCB Capability．],
  [`2`], [`PROCESS_ADDRESS_SPACE`], [InitのRoot Address Space Capability．],
  [`3`], [`PROCESS_ROOT_NODE`], [Init Root Node自身への再帰参照．],
  [`4`], [`PROCESS_PAGE_TABLE_NODE`], [128 SlotのNode．Init用Page Table Capabilityを格納する．],
  [`5`], [`PROCESS_FRAME_NODE`], [32768 SlotのNode．Init Image Frame Capabilityを格納する．],
  [`6`], [`PROCESS_IPC_BUFFER_FRAME`], [Init IPC Bufferを含むFrame Capability．],
  [`7`], [`GENERIC_NODE`], [128 SlotのNode．初期Generic Capabilityを格納する．],
  [`8`], [`INTERRUPT_REGION`], [全IRQからInterrupt Portを作成できるInterrupt Region Capability．],
  [`9`], [`IO_PORT`], [全I/O Address Rangeを持つI/O Port Capability．Driver用の範囲は`MINT`で作る．],
)

Root Slot 4，5，7はNode Capabilityである．Root Slot 6のFrameはRoot Slot 5配下にある同じIPC Buffer FrameのSiblingである．PCB内部の`buffer_frame`も同じFrameを参照する．

== Address Space and Hardware Context

Kernelは新しいAddress Spaceを作成し，Init ImageのFrameをVirtual Address 0から順にMapする．作成した中間Page TableはRoot Slot 4配下へ，Init ImageのFrameはRoot Slot 5配下へ格納する．

Kernel共通のBoot処理はInstruction Pointerを`entry_point_address`へ設定するが，Stack Pointerを明示的に設定しない．初期Stack PointerはHALが作成するUser Hardware Contextに依存する．Initは，CまたはC++のPrologueを実行する前に，Assembly Entry StubまたはHALの初期Contextで有効なStack Pointerを確立する必要がある．Stack用MemoryはInit Imageに含め，対応するFrame数を`init_image_size`へ加える．

InitのPriorityは31，Quantumは10である．KernelはInitをReady Queueへ追加し，最初のUser Processとして実行する．CPU AffinityはBoot経路で明示設定されない．

IPC Buffer Frameは，`loaded_address + init_ipc_buffer_address`と一致するInit Frameから検出される．OffsetがFrame先頭と一致しない場合，KernelはPCBの`process.buffer`とRoot Slot 6を構成しない．Init ImageはIPC Bufferを初期Frameの境界へ配置する必要がある．

中間Page Tableを準備するLoopは，Virtual Address 0から`init_image_size * 2^INITIAL_FRAME_SIZE_BITS`までを終端値を含めて走査する．終端値は最後のImage Frameの直後を指すため，MappingされるFrame数とは別に，直後のAddressに対応する中間Page Tableが追加で作成される場合がある．

== Generic Descriptor Generation

Kernelは，`RESERVED`以外のMemory Map EntryをGenericへ変換する．`FREE` EntryからはDevice Flagが0のGenericを，`DEVICE` EntryからはDevice Genericを作る．2の冪でないEntryは，先頭から格納できる最大の2の冪へ分割し，残りを最大7回まで追加分割する．

Memory Map Entry先頭は`PAGE_SIZE`に合わせたAlignmentが必要である．`memory_map_count > 128`はBoot Failureとなる．対象Revisionは分割後の総Descriptor数を128以下へ制限しない．分割後Descriptor総数が129以上になるMemory Mapは，`generic_list`外への書込みを起こし得る．Bootloaderは分割後Descriptor総数を128以下へ制限する必要がある．

対象Revisionは，各Memory Map Entryの最初のDescriptorを作った直後に`generic_list_count`を更新する．追加分割後には更新しないため，最後の非Reserved Entryから作られた追加DescriptorはCountに含まれない．KernelはCountに含まれるDescriptorだけをGeneric Nodeへ設定する．最後の非Reserved Entryが2の冪Sizeでない場合，追加DescriptorがGeneric Nodeへ反映されない．Bootloaderは，最後の非Reserved Entryを2の冪Sizeにする必要がある．

== Init Startup Requirements

Init Entry Stubは，Kernel Call開始前に次の状態を確立する必要がある．

+ CまたはC++のPrologueが要求するStack Pointerを，Init Image内のMap済みStack上端へ設定する．
+ Link時に確定したAddressから`init_info`を取得する．
+ `init_info.ipc_buffer`からIPC Buffer Pointerを取得する．
+ `generic_list_count`とGeneric Node Slotを照合してResource台帳を作成する．
+ Root Slot 7のGenericから，必要なNode，PCB，Address Space，Page Table，Frame，IPC Port，Notification Portを作成する．

Kernel Callの成否は，`MR0`のFlagと`MR1`のCapability Errorで判定する．レジスタへ収まらないMessage Registerを使う操作は，Init IPC Bufferが構成済みであることを前提とする．

== Boot Failure Model

不正な`boot_info` Pointer，0 Address，Alignmentの不一致，Page Table作成の失敗，Init Allocatorの不足，Image Frame Mappingの失敗，Generic Descriptorの不整合は，Init作成の失敗やKernel停止につながる．BootloaderへErrorを返す経路はない．Bootloaderは，Kernelへ制御を渡す前に全Fieldと物理範囲を検証する必要がある．
