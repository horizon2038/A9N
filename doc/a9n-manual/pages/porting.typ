#import "/components/reference.typ" : reference_table, fields, notice

= HAL Porting Guide

== Scope and Starting State

HALを移植するには，新しいArchitecture用のBoot，Context，Memory Mapping，Interrupt，Timer，I/O，Kernel Call Entryを実装し，共通のKernel Interfaceへ接続する．対象Revisionで起動まで実装されているArchitecture Directoryは`src/hal/x86_64`だけである．

移植には，Freestanding C++20 Toolchain，Target用AssemblerとLinker，Boot環境，Architecture Manual，Interrupt ControllerとTimerの仕様が必要となる．Kernelの初期化，Initの起動，Capability Call，Timer Preemption，IPC，Fault配送がTarget HardwareまたはEmulator上で動作すれば，最小のPortが成立する．

== Source Layout

新しいArchitecture名を`{ARCH}`とする．Directoryは次のように構成する．

```text
src/hal/{ARCH}/
├── CMakeLists.txt
├── toolchain.cmake
├── kernel.ld
├── include/hal/arch/arch_types.hpp
├── boot/
├── arch/
├── process/
├── systemcall/
├── memory/
├── interrupt/
├── time/
├── io/
└── factory/
```

Directory名はCMakeの`ARCH`値と一致させる．Root CMakeが`src/hal/${ARCH}`の存在を確認し，`src/hal/CMakeLists.txt`がSubdirectoryを追加する．Kernel Targetには`src/hal/${ARCH}/include`をInclude Pathとして渡す．

#notice(
  [CAUTION],
  [`src/hal/CMakeLists.txt`の一つのInclude Pathは`${arch}`というLowercase変数を参照する．標準入力変数は`${ARCH}`である．新しいPortでArchitecture Headerが見つからない場合，Lowercase参照を修正する必要がある．],
)

== Architecture Constants

`include/hal/arch/arch_types.hpp`はKernel Data StructureとABI Sizeを決める．少なくとも次の定数を定義する必要がある．

#reference_table(
  (1.8fr, 2.8fr),
  ([*Constant*], [*Description*]),
  [`BYTE_BITS`], [1 ByteのBit数．A9N共通型は8を前提とする．],
  [`PAGE_SIZE`], [基本Page Size．Kernel共通実装が仮定する値と一致させる．],
  [`KERNEL_VIRTUAL_BASE`], [Physical MemoryのKernel Direct Map Base．],
  [`USER_VIRTUAL_BASE`], [User Virtual Address Base．],
  [`HARDWARE_CONTEXT_SIZE`], [PCB Hardware ContextのWord数．Register Read/Write ABIとFault ABIへ露出する．],
  [`FLOATING_CONTEXT_SIZE`], [Floating-point ContextのWord数．],
  [`VIRTUAL_CPU_CONTEXT_SIZE`], [Virtual CPU Context Buffer Size．Virtualization未完成でもBuildに必要である．],
  [`VIRTUAL_CPU_STATE_COUNT`], [Virtual CPU State Descriptor数．],
  [`IRQ_NUMBER_MAX`], [Kernel IRQ Handler TableのEntry数．],
  [`INITIAL_FRAME_SIZE_BITS`], [Init ImageをMapするFrame Size Bits．BootloaderのImage Size単位と一致させる．],
)

Kernel共通Codeには，Page Size，Word幅，DescriptorのDepth幅，Kernel Direct Mapを固定値として扱う処理がある．異なる値を採用するPortでは，HALだけでなくKernel共通ABIも変更する必要がある．対象Revisionが仮定する値は「x86_64 ABI」に記載する．

== Required HAL Interfaces

#reference_table(
  (1.5fr, 1.8fr, 2fr),
  ([*Area*], [*Interface Header*], [*Required responsibility*]),
  [Architecture Init], [`arch_initializer.hpp`], [`arch_info[]`をDecodeし，CPU，Interrupt，Timer，Platformを初期化する．],
  [CPU-local State], [`cpu.hpp`，`lock.hpp`], [Core番号，CPU-local Pointer，Kernel Lockを提供する．],
  [Process Context], [`process_manager.hpp`], [Context初期化，Context Switch，Restore，Message Register，General Register，User Address検査を実装する．],
  [Memory], [`memory_manager.hpp`], [Address Space作成，Page TableとFrameのMap／Unmap，Depth探索，Frame Size検査を実装する．],
  [Interrupt], [`interrupt.hpp`], [Timer，Kernel Call，IRQ，Fault Handler登録とIRQ Mask／Unmask／Ackを実装する．],
  [Timer], [`timer.hpp`], [System Clock Frequency設定を実装する．],
  [Port I/O], [`port_io.hpp`], [Architectureが持つI/O SpaceのRead／Writeを実装する．MMIO ArchitectureはCompatibility方針を定義する．],
  [Serial], [`serial.hpp`], [Early Boot Logger用Serial Driverを実装する．],
  [Factory], [`hal_factory.hpp`], [Architecture固有HAL Objectを生成する．],
  [Virtualization], [`virtualize.hpp`], [Buildに必要なStubまたはHardware Virtualization実装を提供する．],
)

== Kernel Call Port

Kernel Call Entryは，ArchitectureのPrivilege Transition命令から`kernel_call_handler`を呼ぶ．Kernel Call NumberはSigned Wordとして渡す．Message Registerは，`get_message_register()`と`configure_message_register()`から読み書きできるようにする．

Registerへ収まらないMessage RegisterはIPC Bufferに割り当てる．Registerとの対応，破壊されるRegister，保存されるRegister，Return命令，Stack Alignment，Interrupt MaskingをArchitecture ABIに記載する必要がある．

不正Kernel Call Numberは`fault_dispatcher(INVALID_KERNEL_CALL, ...)`へ渡す．Fault Address，Program Counter，Architecture Fault Codeの意味はArchitecture Portが定義する．

Context Switchでは，Hardware Context，Floating-point Context，Thread-local Base，Address Spaceを切り替える．同じAddress Space間でRoot Registerの再設定を省略してもよいが，TLBの整合性を保つ必要がある．

== Memory Port

Memory HALは，Address Space Rootの検査，Page Table Depth，Frame Size，Mappingの重複，Unmapping，TLB Invalidationを実装する．KernelはHAL ErrorをCapability Errorへ変換するため，`memory_map_error`の意味を変えてはならない．

`make_address_space()`は，新しいUser Address SpaceへKernel Mappingを組み込む．Kernel MappingをUser Modeから参照できないようにPage EntryのPrivilegeを設定する．`is_valid_user_address()`の判定範囲は，User Mappingを作成できる範囲と一致させる．

SMP Portは，Core列挙と起動，CPU-local Pointer，Core数，Reschedule IPI，TLB Shootdown IPI，CoreごとのTimerとSingle Kernel Stackを実装する必要がある．Kernel共通CodeはGiant LockとCoreごとのSchedulerを提供する．Memory HALのAddress Space Owner APIは，Capability Dependencyをまたいで共有できるArchitecture固有領域へOwner Bitmapを保存し，Bitmap全体の読み書きだけを提供する．Core BitのSet／ClearはArchitecture固有Context Switchが直接行い，IPI Targetの選択と送信はKernelが行う．Context Switchでは，次のAddress Spaceへ現在CoreのBitをSetしてからHardware Rootを切り替え，保存した以前のHardware RootからBitをClearする．

== Interrupt and Timer Port

Architecture ExceptionはA9Nの`fault_type`へ変換する．Memory FaultにはFault AddressとInstruction-fetchの区分を付ける．Invalid Instruction，Arithmetic Fault，Invalid Kernel Call，Architecture Faultは，Resolver IPCへ配送できる情報へ変換する．回復できないExceptionは`FATAL`として扱う．

外部IRQは0から`IRQ_NUMBER_MAX - 1`までのKernel IRQ番号へ正規化する．Timer IRQはProcess Managerの`handle_timer()`へ接続する．外部IRQはNotification後にMaskし，Interrupt Portの`ACK`でUnmaskする．

== Boot Protocol Port

Boot Entryは`boot_info*`をKernelへ渡す．BootloaderとHALは，`arch_info[]`のIndexごとの意味を共有する必要がある．Init Imageは`INITIAL_FRAME_SIZE_BITS`単位で，User Address 0からMapされる．Bootloaderが用いるFrame Sizeも同じ値にする．

新しいArchitectureのBuildは，PointerのSizeとAlignmentが1 Wordであること，`memory_map_type`が4 Byte，`bool`が1 Byteであることを検査する必要がある．`offsetof`と`sizeof`によるBuild-time Assertionは，「Boot and Init Protocol」のWord単位のLayoutと`boot_info`，`memory_info`，`memory_map_entry`，`init_image_info`，`generic_descriptor`，`init_info`の実Layoutを照合する．Layoutが一致しない場合，BootloaderとKernelで同じStructureを共有してはならない．

Kernel Linker ScriptはKernel Virtual Address，Physical Load Address，Boot Section，Page Table，Stack，Entry Symbolを定義する．Boot AssemblyはKernel Direct MapとKernel Stackを確立してからC++ Entryへ移る必要がある．Global Constructorが実行される保証はないため，Early Boot ObjectはStatic StorageとPlacement Newを使用する．

== Build and Verification

Docker Buildの実行例は次の通りである．`{ARCH}`はArchitecture Directory名へ置き換える．

```sh
ARCH={ARCH} BUILD_TYPE=Release docker compose run --rm a9n-build
```

Local CMake Buildの実行例は次の通りである．

```sh
cmake -S . -B build-{ARCH} \
  -DARCH={ARCH} \
  -DCMAKE_TOOLCHAIN_FILE=./src/hal/{ARCH}/toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build-{ARCH}
```

Build成功時は`kernel.elf`と`kernel.map`がBuild Directoryに生成される．Build成功だけではPort完了としない．Portは次の順で検証する．

+ Serial LoggerがKernel EntryとBoot情報を出力する．
+ Architecture，Interrupt，Process Managerの初期化が完了する．
+ InitがUser Modeで実行され，有効なStackと`init_info`を取得する．
+ `DEBUG` Callと`YIELD` CallがReturnする．
+ Capability Descriptorを構成し，Init Root Capability NodeのSlotを探索できる．
+ GenericからNode，Frame，Address Space，PCB，IPC Portを作成できる．
+ Address SpaceへPage TableとFrameをMapし，User MemoryへAccessできる．
+ IPC Portの`CALL`／`REPLY_RECEIVE`がPayloadとCapabilityを転送する．
+ Notificationと外部IRQがDriverへ配送され，`ACK`で再Enableされる．
+ User FaultがResolver IPCへ配送され，Reply後にProcessが再開する．
+ Timer Tickが同Priority ProcessをRound-robinで切り替える．

Test HALの`src/hal/test/include/hal/arch/arch_types.hpp`はHost Unit Test用であり，Hardware Portの代替ではない．Architecture Portは，Register ABI，Page Table，Interrupt Entry，Context RestoreをTarget Emulatorまたは実機で検証する必要がある．
