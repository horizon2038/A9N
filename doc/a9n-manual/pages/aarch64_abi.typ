#import "/components/reference.typ" : reference_table, notice, term

= AArch64 ABI

== Scope

#term[AArch64 ABI]は，A9Nの共通Kernel InterfaceをAArch64へ対応させる規約である．本章では，Kernel Call，Hardware Context，ExceptionとFault，Page Table，Boot Protocol，U-Bootの順にAArch64固有の規約を示す．Architecture Directory名とBuild識別子には正式名称の`aarch64`を使用する．

#reference_table(
  (1.6fr, 2.8fr),
  ([*項目*], [*値*]),
  [Build選択], [`-DARCH=aarch64`．],
  [Word幅], [64 bit．],
  [Byte Order], [little-endian．],
  [基本Page Size], [4 KiB．`PAGE_SIZE = 4096`，`INITIAL_FRAME_SIZE_BITS = 12`．],
  [User Address], [`0 <= address < 0x0000_8000_0000_0000`．上限値は含まれない．],
  [Kernel Direct Map], [物理Addressに`KERNEL_VIRTUAL_BASE = 0xffff_8000_0000_0000`を論理和したVirtual Addressを用いる．],
  [IRQ数], [`IRQ_NUMBER_MAX = 256`．A9Nが公開するIRQ番号は0から255である．],
)

BoardまたはEmulator固有実装は`-DPLATFORM={PLATFORM}`で選択し，`src/hal/aarch64/platform/{PLATFORM}`へ配置する．対象Revisionで実装されているPlatformは`qemu`である．Architecture共通部はEL Transition，Page Table，Context，A9N Boot Protocol構築を担当する．Platform部はEntry，Serial，Interrupt Controller，TimerなどHardware依存処理を担当する．

`main.cpp`は`hal/interface/hal.hpp`だけをHALの入口として使用する．HALはRuntime FactoryやVirtual Dispatchを持たず，`ARCH`と`PLATFORM`によってLink時に実装を選択する．

```text
src/hal/aarch64/
├── arch/
├── boot/
├── interrupt/
├── io/
├── memory/
├── process/
├── virtualization/
└── platform/
    └── qemu/
        ├── boot/
        ├── interrupt/
        ├── io/
        └── time/
```

== Kernel Call ABI

EL0からのKernel Callには`svc #0`を用いる．`x8`へSigned 64 bitのCall Typeを置き，ArgumentとResultはVirtual Message Registerで渡す．Exception Handlerは`ESR_EL1.EC = 0x15`をAArch64からのSVCとして認識し，保存した`x8`を`kernel_call_type`としてKernelへ渡す．SVC ImmediateはCall Typeとして使用せず，0でなければならない．

#reference_table(
  (1.4fr, 0.8fr, 2.4fr),
  ([*Call Type*], [*`x8`*], [*意味*]),
  [`CAPABILITY_CALL`], [`-1`], [Capability DescriptorとObject OperationをVirtual Message Registerで渡す．],
  [`YIELD`], [`-2`], [実行中ProcessがCPUを譲る．],
  [`DEBUG`], [`-3`], [Deprecatedな1文字出力．`MR0`の下位8 bitを使用する．],
)

`x8`はKernel Call Type専用であり，Message Registerではない．`MR0`から`MR7`は`x0`から`x7`へ連続して割り当て，その次は`x8`を飛ばして`x9`と`x10`を使用する．`MR10`以降は，実行中Processに設定されたIPC Bufferの`messages[index]`を使用する．

#block(breakable: false)[
  #reference_table(
    (1.2fr, 2.2fr),
    ([*ABI上の値*], [*保存先*]),
    [Kernel Call Type], [`x8`],
    [`MR0`], [`x0`],
    [`MR1`], [`x1`],
    [`MR2`], [`x2`],
    [`MR3`], [`x3`],
    [`MR4`], [`x4`],
    [`MR5`], [`x5`],
    [`MR6`], [`x6`],
    [`MR7`], [`x7`],
    [`MR8`], [`x9`],
    [`MR9`], [`x10`],
    [`MR10..MR493`], [`ipc_buffer.messages[index]`],
  )
]

`MESSAGE_BUFFER_SIZE_MAX`は494である．HALはMR Indexの範囲を検査しないため，呼出し側は$i < 494$を満たすことを確認する必要がある．`MR10`以降を使用するCallには，PCBへ設定済みのIPC Bufferが必要である．ユーザ空間の`a9n_abi`は`TPIDR_EL0`をIPC Buffer Pointerとして読み出すため，通常のWrapperを使う前にThread Local Baseも設定する．Initが最初に設定するときは，現在のIPC Bufferの`MR10`へPointerを書き，`a9n_abi 0.6.0`の`early_configure_to_tls`相当のPCB `CONFIGURE`を直接実行する．

`CAPABILITY_CALL`の共通Register Layoutは次の通りである．Operation固有のLayoutは各Kernel Objectの章に記載する．

#reference_table(
  (1fr, 1.2fr, 2.5fr),
  ([*方向*], [*MR*], [*値*]),
  [入力], [`MR0` / `x0`], [Raw Capability Descriptor．],
  [入力], [`MR1` / `x1`], [Object Operation Number．],
  [入力], [`MR2..`], [Operation固有Argument．],
  [出力], [`MR0` / `x0`], [成功時1，失敗時0．],
  [出力], [`MR1` / `x1`], [失敗時の`capability_error`．成功時の値は未定義．],
  [出力], [`MR2..`], [Operation固有Result．],
)

Capability Errorの値は`ILLEGAL_OPERATION = 0`，`PERMISSION_DENIED = 1`，`INVALID_DESCRIPTOR = 2`，`INVALID_DEPTH = 3`，`INVALID_ARGUMENT = 4`，`FATAL = 5`，`DEBUG_UNIMPLEMENTED = 6`である．`MR0 != 0`だけが成功を表す．成功時に`MR1`が0になるとは仮定しない．

Exception Entryは`x0..x30`を保存し，Kernelが選択したProcessのContextを`eret`直前に復元する．したがって，Kernel Objectが出力として更新したMR以外のGeneral-purpose RegisterはTrap境界で保存される．ただし，Inline Assembly Wrapperから見た保存・破壊規約は使用言語のABIにも従う．`x8`はTrap Selectorであり，呼出し側の一時値の保存場所として使用してはならない．`YIELD`と`DEBUG`にはResult Registerを定義しない．

== Hardware Context ABI

PCBの`READ_REGISTER`と`WRITE_REGISTER`が扱うHardware Contextは35 Wordである．`INVALID_KERNEL_CALL` Fault MessageへHardware Contextを含める場合も同じ並びを使用する．

#reference_table(
  (0.9fr, 1.3fr, 2.5fr),
  ([*Index*], [*Register*], [*用途*]),
  [`0..30`], [`x0..x30`], [AArch64 General-purpose Register．IndexとRegister番号が一致する．],
  [`31`], [`SP_EL0`], [EL0 Stack Pointer．],
  [`32`], [`ELR_EL1`], [Exceptionから復帰するProgram Counter．],
  [`33`], [`SPSR_EL1`], [復帰時PSTATE．User ContextのModeは`EL0t = 0x0`．],
  [`34`], [`TPIDR_EL0`], [Thread Local Base．`a9n_abi`ではIPC Buffer Pointerとして使用する．],
)

HALのArchitecture-independent Register Typeは，`INSTRUCTION_POINTER`を`ELR_EL1`，`STACK_POINTER`を`SP_EL0`，`THREAD_LOCAL_BASE`を`TPIDR_EL0`へ対応させる．新しいUser ProcessのContextは0で初期化し，`SPSR_EL1.M`だけを`EL0t`へ設定する．Kernel Contextでは`EL1h = 0x5`を使用する．

Floating-point Contextは528 byteである．Offset 0から511へ`q0..q31`を各16 byteで並べ，Offset 512へ`FPCR`，Offset 520へ`FPSR`を各64 bitで格納する．SchedulerはContext Switch時にこの領域を保存・復元するが，PCBの`READ_REGISTER`と`WRITE_REGISTER`が扱う35 WordのHardware Contextには含まれない．

#notice(
  [CAUTION],
  [`WRITE_REGISTER`は`ELR_EL1`，`SPSR_EL1`，`SP_EL0`，`TPIDR_EL0`の値を検証しない．Process ManagerとFault Resolverは，Program CounterとStack PointerがUser Address範囲にあり，`SPSR_EL1`がEL0へ復帰する値であることを確認してからContextを書き戻す必要がある．],
)

#notice(
  [CAUTION],
  [実行中Processが自身のPCBへ`READ_REGISTER`を実行する場合，`register_count`は4未満にする必要がある．`register_count >= 4`では，Index 0の返却先`MR3`がHardware ContextのIndex 3（`x3`）を上書きしてからIndex 3を読むため，Index 3以降は呼出し開始時点のSnapshotにならない．35 WordのContextを確認するProcess Managerは，対象Processを停止して別Processから`READ_REGISTER`を実行する必要がある．],
)

== Exception and Fault ABI

Exception Vector Tableは2 KiB Alignmentを持ち，各Vector Slotは128 byteである．Current ELのSynchronous ExceptionとIRQ，Lower ELのAArch64 Synchronous ExceptionとIRQを処理する．Lower ELのAArch32 Vector，FIQ，SError，未対応のCurrent EL ExceptionはFatalとなる．

Exception EntryがKernel Stackへ作るFrameは304 byteである．先頭の35 WordはHardware Contextと同じ順序であり，その後へ`ESR_EL1`，`FAR_EL1`，予約Wordを格納する．この304 byte FrameはHAL内部形式であり，ユーザ空間へそのままCopyする構造体ではない．Fault IPCは「Kernel Call」の共通Fault Message Layoutに従う．

#reference_table(
  (1.3fr, 1fr, 2.4fr),
  ([*ESR_EL1.EC*], [*A9N Fault*], [*AArch64固有値*]),
  [`0x15`], [Kernel Call], [`x8`のCall TypeをDispatchする．Fault IPCにはしない．],
  [`0x20`], [`MEMORY_INSTRUCTION_FETCH`], [`MR5 = ELR_EL1`，`MR6 = FAR_EL1`，`MR7 = ESR_EL1`．],
  [`0x24`], [`MEMORY`], [`MR5 = ELR_EL1`，`MR6 = FAR_EL1`，`MR7 = ESR_EL1`．],
  [`0x00`], [`INVALID_INSTRUCTION`], [`MR5 = ELR_EL1`，`MR6 = ESR_EL1`．],
  [その他], [`ARCHITECTURE`], [`MR5 = ELR_EL1`，`MR6 = ESR_EL1`．],
)

IRQ HandlerはGICから返されたInterrupt IDが1020未満の場合だけDispatchする．System Timer，Reschedule SGI，TLB Invalidate SGIはHAL内部Handlerへ渡し，その他のIRQはArchitecture-independent Interrupt Dispatcherへ渡す．Device DriverがInterrupt Portの`ACK`を呼ぶと，HALは現在ActiveなIRQへEnd-of-interruptを発行する．

== Page Table ABI

AArch64のUser Address Spaceは4 KiB Granule，4階層，1 Tableあたり512 Entryで構成する．User Mappingは`TTBR0_EL1`，Kernel Mappingは共有する`TTBR1_EL1`を使用する．Address Spaceを作成するとHALはRoot Tableを0で初期化し，Kernel MappingをCopyしない．

#reference_table(
  (1fr, 0.8fr, 1fr, 2.3fr),
  ([*Level*], [*Depth*], [*範囲*], [*役割*]),
  [`L0`], [`4`], [512 GiB], [Address SpaceのRoot．],
  [`L1`], [`3`], [1 GiB], [中間Table，または1 GiB BlockのLeaf．],
  [`L2`], [`2`], [2 MiB], [中間Table，または2 MiB BlockのLeaf．],
  [`L3`], [`1`], [4 KiB], [4 KiB PageのLeaf．],
)

Frame Size Bitsは12，21，30であり，それぞれL3 Page，L2 Block，L1 Blockへ対応する．指定したFrame SizeにVirtual AddressとPhysical Addressの両方をAlignする必要がある．HALはAlignmentを検査する．Page Table Capabilityが指定するDepthの有効範囲は1から3，Address Space RootのDepthは4である．

L0[511]は通常のMappingに使用せず，Address Spaceを実行中のCoreを表すOwner BitmapとしてHALが予約する．このEntryが対応する範囲はUser Address上限の外側である．Context Switchは`TTBR0_EL1`を切り替え，共有Kernel Mappingの`TTBR1_EL1`を維持する．現在のAddress SpaceのMappingを変更した場合は`TLBI VMALLE1IS`をBarriersとともに実行する．

#reference_table(
  (1fr, 0.9fr, 2.7fr),
  ([*Right*], [*値*], [*AArch64 Descriptor*]),
  [`READ`], [`0x1`], [独立したRead-disable Bitはない．User Access可能なNormal Memory Mappingを作る．],
  [`WRITE`], [`0x2`], [未指定ならAP Read-only Bitを設定する．],
  [`EXECUTE`], [`0x4`], [未指定ならUXNを設定する．Kernelからの実行は常にPXNで禁止する．],
)

Leaf DescriptorにはValid，Access Flag，Inner Shareable，Normal Memory Attribute，User Access，PXNを設定する．L3だけPage Bitを設定し，L1とL2はBlock Descriptorを使用する．

== Boot ABI

=== Entry Contract and Boot Chain

A9Nの標準AArch64 BootはU-BootをSecondary Bootloaderとして使用する．標準Boot Chainは次の順序である．

+ FirmwareまたはBoard固有の前段がU-Bootを起動する．
+ U-BootがA9NのAArch64 Image，Nun ELF，実行対象BoardのDTBをMemoryへ配置する．
+ U-Bootが`booti <kernel> <initrd-address>:<initrd-size> <dtb>`を実行する．
+ U-Bootは最終DTBのPhysical Addressを`x0`へ設定してA9N Image Entryへ制御を移す．
+ AArch64 EntryがExceptionをMaskし，必要ならEL2からEL1へ遷移し，BSS，Boot Stack，Page Table，MMUを初期化する．
+ Pre-entryの`aarch64_prepare_boot_protocol(x0)`がDTBとNun ELFからA9N Boot Protocolを構築する．
+ A9N Kernel Entryへ`boot_info*`を`x0`で渡す．

Image Entryが受理するException LevelはEL2またはEL1である．EL2から開始した場合はAArch64 EL1，`SPSR_EL2.M = EL1h`へ遷移する．それ以外のELでは停止する．Pre-entryより前はMMUを有効化し，物理AddressのIdentity Mapと`0xffff_8000_0000_0000`からのHigher-half Direct Mapを一時的に使用する．Architecture初期化後は一時的な`TTBR0_EL1` Identity Mapを破棄する．

`kernel.img`は64 byteのLinux AArch64 Image Headerを持つraw Imageである．Headerの`text_offset`はPhysical Load BaseからEntryまでのOffsetを示し，`image_size`はFile SizeではなくBSSを含む実行時占有範囲を示す．BootloaderはInitrdとDTBをこの範囲へ重ねてはならない．

#notice(
  [CAUTION],
  [QEMUの`-kernel`でA9Nを直接起動する経路はPre-entryの単体検証には利用できるが，標準Boot Chainではない．Board移植と同じ条件を検証する場合は，U-Bootから`booti`を実行する．],
)

=== A9N Boot Protocol Construction

Pre-entryはDTB HeaderとStructure Blockを検証し，`/memory`，Memory Reservation Block，`/reserved-memory`を収集する．`/chosen`の`linux,initrd-start`と`linux,initrd-end`はU-Bootが配置したNun ELFの範囲である．Pre-entryはAArch64 ELFのLoad SegmentをKernel内のInit Image領域へ展開し，Entry Point，`__init_info_start`，`__init_ipc_buffer_start`から`boot_init_image_info`を設定する．

#reference_table(
  (1.8fr, 2.8fr),
  ([*Boot field*], [*AArch64 meaning*]),
  [`boot_info.arch_info[0]`], [U-BootがImage Entryの`x0`で渡した最終DTBのPhysical Address．],
  [`boot_info.arch_info[1..127]`], [予約．0に初期化する．],
  [`boot_init_image_info`], [DTBのinitrd範囲にあるNun ELFを展開したInitial Process Image．],
  [`boot_memory_info`], [DTBのRAM範囲からKernel，DTB，initrd，Memory Reservation，`reserved-memory`を除外して構築したMemory Map．],
)

Kernel初期化後も`arch_info[0]`はDTBのPhysical Addressを表す．HALはこれをDirect Map Addressへ変換してPlatform初期化に使用する．DTB Address 0，壊れたFDT Header，または有効なinitrd範囲を持たないBootからはInitを生成できない．

=== U-Boot Boot Media

SPENCERのAArch64 Image Builderは，MBRとActiveな第1 FAT32 Partitionを持つDisk Imageを生成する．Partitionを持たないFAT superfloppyは，U-Boot Standard BootがBoot Deviceを認識してもScript Bootflowを作成できない場合があるため使用しない．

#reference_table(
  (1.7fr, 2.8fr),
  ([*Location*], [*Purpose*]),
  [MBR Partition 1], [LBA 2048から始まるActive FAT32 LBA Partition．],
  [`/boot.scr`], [U-Boot legacy script image．Script bootmethがRootから検出する．],
  [`/boot/boot.scr`], [`/boot` Prefixを探索するU-Boot向けの同一Script．],
  [`/boot.cmd`], [生成元の可読Script．診断と手動実行に用いる．],
  [`/kernel/kernel.img`], [Linux AArch64 Image Headerを持つA9N Kernel Image．],
  [`/kernel/init.elf`], [`booti`のraw initrdとして渡すNun AArch64 ELF．],
)

legacy `boot.scr`のPayloadは，単なるScript Textではない．Payload先頭にbig-endianのScript Size，続いてzero terminatorを置き，その後にScript Textを配置する．HeaderのData SizeとData CRCは，Size Tableを含むPayload全体を対象とする．

Script bootmethはScript実行前に`devtype`，`devnum`，`distro_bootpart`を設定する．Boot Scriptは次の形式でFileを読むため，virtio，MMC，USBなど特定のDevice Classへ依存しない．

```text
load ${devtype} ${devnum}:${distro_bootpart} <address> <path>
```

`kernel_addr_r`，`ramdisk_addr_r`，`fdt_addr_r`はBoardのU-Boot Environmentに定義された値を優先する．未定義の場合だけQEMU `virt`用の既定値を設定する．Boot ScriptはAArch64 Image HeaderのOffset `0x10`から`image_size`を読み，Initrd AddressがKernel占有範囲と重なる場合はKernel終端より2 MiB後方へ移動する．

DTB Sourceは`fdt_addr`を優先する．これが未定義の場合は`fdtcontroladdr`を使用する．Source DTBの`totalsize`に更新用余白を加えて`fdt_addr_r`へCopyし，そのAddressを`booti`へ渡す．したがって，FirmwareまたはBoard Portは実行対象Hardwareを記述するDTBをU-Bootへ提供する必要がある．

U-Bootにはlegacy script image，filesystem `load`，`setexpr`，`itest`，`fdt`，`booti`が必要である．自動探索を用いるBuildでは，Standard BootとScript bootmethも有効にする．自動探索を持たないBoardでも，Partition 1から`boot.scr`をLoadして`source`を実行すれば同じBoot ABIを使用できる．

=== Adding an AArch64 Board Platform

新しいBoard `{BOARD}`は`src/hal/aarch64/platform/{BOARD}`へ追加し，`-DPLATFORM={BOARD}`で選択する．AArch64共通のU-Boot，DTB，A9N Boot Protocolは再利用し，次をBoard固有実装とする．

+ U-Bootを開始するFirmwareまたはSPLとの接続．
+ Kernel Linker ScriptのPhysical Load AddressとImage Header．
+ Early Serial，Interrupt Controller，Generic TimerまたはBoard Timer．
+ MMIO Range，IRQ番号，CPU Topology，Secondary CPU Boot Method．
+ Board DTBが表すRAMとReserved Memoryの検証．

SPENCER上でQEMUを用いて標準Boot Chainを検証するCommandは次の通りである．SPENCER RepositoryのRootで実行し，`UBOOT_BIN`にはQEMU `virt`用AArch64 U-Bootを指定する．

```sh
UBOOT_BIN=/path/to/u-boot.bin cargo xtask run \
  --arch aarch64 \
  --platform qemu \
  --release
```

成功時はU-Bootの`script` Bootflow，`Starting kernel ...`，A9Nの`[KERNEL]`と`[HAL]`，Nunの起動を順に確認できる．
