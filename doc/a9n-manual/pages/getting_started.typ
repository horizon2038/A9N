#import "/components/reference.typ" : reference_table, notice, term
#import "@preview/cetz:0.5.2"

= Getting Started

== Goal

本章では，#term[SPENCER]が用意する標準構成をBuildし，A9N Microkernel上でUser Payloadが動作するところまでを確認する．標準構成は，#term[A9NLoader-rs]をBootloader，#term[Nun]をUser-level Runtime，SPENCERの`core` PackageをUser Payloadとして用いる．Nunは#term[`a9n_abi`]を介してKernel Interfaceを利用し，`a9n_abi`は共有型を定義する#term[`a9n_types`]へ依存する．

// TODO: `--platform qemu`は，生成したDisk Imageがx86_64 UEFI実機でも起動できるという実態と一致しない．SPENCER側でplatform名を修正した後，本章のCommand，Path，説明を更新する．
Version #(read("/version.txt").trim())のSPENCER CLIは，x86_64では`--platform qemu`，aarch64では`--platform qemu`または`--platform rpi4b`を受理する．Getting Startedでは各Interfaceの内部を変更せず，標準構成のBootを先に確認する．生成したx86_64 Disk ImageはQEMUだけでなくUEFI実機でも起動できる．aarch64のKernel Call，Context，Page Table，U-Boot経路とRaspberry Pi 4 Model B実機Bootは「AArch64 ABI」に記載する．Capabilityを用いたServiceの構成は「Building Init and Services」，複数Coreの構成は「Symmetric Multiprocessing」，x86_64のRegister配置や独自Entryの実装は「x86_64 ABI」に記載する．

== Execution Path

#figure([
  #set text(size: 8pt)
  #cetz.canvas({
    import cetz.draw: *

    set-style(
      stroke: 0.45pt,
      mark: (transform-shape: false, fill: black),
    )

    content((0, 0), [SPENCER / `cargo xtask`], name: "spencer", frame: "rect", padding: 0.75em, fill: luma(235))
    content((-4.2, -1.8), [A9NLoader-rs], name: "loader", frame: "rect", padding: 0.75em, fill: luma(247))
    content((0, -1.8), [A9N Kernel], name: "kernel", frame: "rect", padding: 0.75em, fill: luma(247))
    content((4.2, -1.8), [`core` User Payload], name: "payload", frame: "rect", padding: 0.75em, fill: luma(247))
    content((0, -3.65), [Bootable Disk Image], name: "image", frame: "rect", padding: 0.75em, fill: luma(235))

    line("spencer.south", (-4.2, -0.9), "loader.north", mark: (end: ">"))
    line("spencer.south", "kernel.north", mark: (end: ">"))
    line("spencer.south", (4.2, -0.9), "payload.north", mark: (end: ">"))
    line("loader.south", (-4.2, -2.75))
    line("kernel.south", (0, -2.75))
    line("payload.south", (4.2, -2.75))
    line((-4.2, -2.75), (4.2, -2.75))
    line((0, -2.75), "image.north", mark: (end: ">"))

    line((-5.3, -4.65), (5.3, -4.65), stroke: gray + 0.2pt)
    content((-5.3, -5.05), [Boot後の実行順], anchor: "west")

    content((-4.3, -6.15), [A9NLoader-rs], name: "run-loader", frame: "rect", padding: 0.75em, fill: luma(247))
    content((-1.45, -6.15), [A9N Kernel], name: "run-kernel", frame: "rect", padding: 0.75em, fill: luma(247))
    content((1.35, -6.15), [Nun Runtime], name: "run-nun", frame: "rect", padding: 0.75em, fill: luma(247))
    content((4.2, -6.15), [`core::main`], name: "run-main", frame: "rect", padding: 0.75em, fill: luma(247))

    line("run-loader.east", "run-kernel.west", mark: (end: ">"))
    line("run-kernel.east", "run-nun.west", mark: (end: ">"))
    line("run-nun.east", "run-main.west", mark: (end: ">"))
    content((-2.88, -5.55), [Kernel Entry], anchor: "south")
    content((-0.05, -5.55), [Init Entry], anchor: "south")
    content((2.78, -5.55), [User Entry], anchor: "south")
  })
], caption: [標準構成のBuild対象とBoot後の実行順])

SPENCERはA9N Kernel，A9NLoader-rs，User Payloadを個別にBuildし，三つのExecutableを一つのDisk Imageへ格納する．Boot時にはA9NLoader-rsがKernelとInit Imageを読み込み，A9N Kernelへ制御を渡す．Kernelは`core`をInitとして構成する．`core`のEntryはNunが初期化し，IPC BufferをRuntimeへ対応付けた後に`main`を呼ぶ．

`a9n_abi`と`a9n_types`は独立したExecutableではない．Cargo DependencyとしてNunとUser PayloadへLinkされ，Entry，Kernel Call Wrapper，`InitInfo`，`IpcBuffer`等を構成する．

== Host Requirements

BuildにはGit，rustup，CMake 3.29以上，LLVM 18以上，NASM，QEMUを用いる．LLVMにはClang，Clang++，LLD，`llvm-config`が必要である．UEFI FirmwareはA9NLoader-rs Submoduleの`tools` Directoryに含まれる．Rust ToolchainはSPENCERの`rust-toolchain.toml`が指定するため，別のVersionを手動で選択しない．初回Buildでは，Rust ToolchainとCargo Packageを取得するためにNetwork接続を用いる．Command例はPOSIX Shellを対象とし，SPENCERとBuild Artifactの保存先には書込み権限が必要である．

#reference_table(
  (1.25fr, 3.4fr),
  ([Tool], [用途]),
  [Git], [SPENCERとSubmoduleを取得する．],
  [rustup / Cargo], [指定されたRust ToolchainでNunとUser PayloadをBuildする．],
  [CMake / LLVM / NASM], [A9N Kernelとx86_64 HALをBuildする．],
  [QEMU], [生成したUEFI Disk Imageを実行する．],
)

== Source Checkout

取得するRepositoryはSPENCERだけである．A9N，Nun，A9NLoader-rsはSPENCERのGit Submoduleに含まれるため，A9Nの別Cloneは不要である．GitHubのSSH Credentialを用いない環境では，Clone CommandのURL置換規則によってSubmodule URLをHTTPSへ置き換える．

```sh
cd /path/to/workspace
git -c url."https://github.com/".insteadOf=git@github.com: \
  clone --recurse-submodules \
  https://github.com/horizon2038/spencer.git spencer
cd spencer
git submodule status
```

`/path/to/workspace`はSPENCERを配置するDirectoryである．`git submodule status`の行頭にある`-`は，行に示されたSubmoduleをまだ取得していない状態を表す．未取得のSubmoduleがある場合は，SPENCER Directoryで`git submodule update`を実行する．

```sh
git submodule update --init --recursive
```

`cargo`にはrustupが管理するExecutableを用いる．Package Managerが導入した別のCargoが先に選択される環境では，rustupのExecutable DirectoryをShellの検索順で先に置く．`command -v cargo`はCargoのPathを表示する．`rustup show active-toolchain`は選択されたToolchainを表示する．`cargo xtask --help`はSPENCERのCommandを表示する．

```sh
command -v cargo
rustup show active-toolchain
cargo xtask --help
```

== Build

SPENCER Directoryで次を実行する．`--os-manifest`を省略すると`core/Cargo.toml`，`--os-target-json`を省略すると`Nun/arch/x86_64-unknown-a9n.json`，`--os-binary`を省略すると`core`が選択される．

```sh
cargo xtask build \
  --arch x86-64 \
  --platform qemu \
  --release
```

`cargo xtask build`は，CMakeによるA9N KernelのBuild，CargoによるA9NLoader-rsのBuild，Cargoによる`core`のBuild，Disk Imageの生成を順に実行する．`core`はNunへ依存し，Nunは`a9n_abi`へ，`a9n_abi`はCargo Package `a9n-types`へ依存する．Build Logの`nun`，`a9n_abi`，`a9n-types`は，Cargoによる依存関係の解決とCompileを示す．Cargo Package名は`a9n-types`，Rustのcrate名は`a9n_types`である．

#reference_table(
  (1.2fr, 3.5fr),
  ([Artifact], [Path]),
  [Kernel], [`out/x86_64-qemu-release/a9n/kernel.elf`],
  [Init], [`out/x86_64-qemu-release/nun_os_target_dir/x86_64-unknown-a9n/release/core`],
  [Loader], [`out/x86_64-qemu-release/a9nloader/a9nloader-rs.efi`],
  [Disk Image], [`out/x86_64-qemu-release/spencer.img`],
)

Disk Image内では，A9NLoader-rsを`/EFI/BOOT/BOOTX64.EFI`，A9N Kernelを`/kernel/kernel.elf`，`core`を`/kernel/init.elf`として配置する．Host上のArtifact名とDisk Image内のFile名を区別する必要がある．

`cargo xtask build`の再実行は，`out/x86_64-qemu-release`以下のArtifactを更新し，`spencer.img`を再生成する．SourceとSubmoduleのRevisionは変更しない．

== Run

Buildと同じSPENCER Directoryで次を実行する．`run`はBuild Pipelineを実行した後，生成したDisk ImageをQEMUで起動する．

```sh
cargo xtask run \
  --arch x86-64 \
  --platform qemu \
  --release
```

Bootが完了すると，Serial出力の末尾に以下の5行が現れる．Version文字列はBuildしたKernelによって異なる．QEMUは`Ctrl-A`に続けて`X`を入力して終了する．

```text
Nun - an operating system framework based on the A9N Microkernel
Configuring <init> ...
Configuring Initial IPC buffer to thread local storage...
Hello, world!
version: <kernel version>
```

Serial出力には，各Componentの境界が実行順に現れる．A9NLoader-rsの出力に続いてA9N Kernelの初期化Logが現れ，NunのLogoとIPC Buffer初期化を経て，`core::main`の`Hello, world!`へ到達する．`Hello, world!`より前のBoot Logは，Loader，Kernel，Runtime，User Payloadの到達段階を示す．

== Run on x86_64 Hardware

`out/x86_64-qemu-release/spencer.img`は，x86_64 UEFI FirmwareがRemovable Mediaとして読み込めるRaw Disk Imageである．Disk Imageは`/EFI/BOOT/BOOTX64.EFI`にA9NLoader-rsを保持する．USBメモリー等のBlock Device全体へDisk ImageをByte単位で書き込むことで，生成物をx86_64実機から起動できる．File System上へ`spencer.img`を通常のFileとしてコピーする操作では，Boot Mediaを構成できない．

#notice([WARNING], [
  Raw Disk Imageの書込みは，指定したBlock Deviceの既存PartitionとDataを上書きする．書込み前に，対象がUSBメモリー等の交換可能Device全体であることをDevice名，容量，接続状態から確認する必要がある．対象Deviceを一意に識別できない場合は，書込みを実行してはならない．
])

Host OSが提供するDisk Image WriterまたはRaw Device Write Toolへ，Imageとして`out/x86_64-qemu-release/spencer.img`，DestinationとしてUSBメモリー等のBlock Device全体を指定する．書込み完了後にHost OSのFlushとDeviceの取外し処理を実行し，x86_64実機のUEFI Boot MenuからUSB Deviceを選択する．Legacy BIOS Bootは対象外である．Secure Bootを有効にしたFirmwareは，署名されていないA9NLoader-rsの実行を拒否し得るため，Firmwareが未署名のUEFI Applicationを許可する設定を用いる必要がある．

実機上の動作範囲は，A9Nのx86_64 HALが対応するCPU，Firmware情報，Interrupt Controller，Timer，I/O Deviceに依存する．USB DeviceからA9NLoader-rsを開始できても，未対応Hardwareを用いるServiceやDriverの動作は保証されない．QEMUと実機は同じDisk Imageを利用できるが，Hardware依存の初期化結果は一致するとは限らない．

== Modify the User Payload

標準のUser Payloadは`core/src/main.rs`にある．`nun::entry!`はNunのEntry Stubを生成し，初期化後に`InitInfo`への参照を`main`へ渡す．`nun::println!`はNunが提供する出力Macroである．最小の変更は，表示する文字列を置き換えて再度`cargo xtask run`を実行することで確認できる．

```rust
#![no_std]
#![no_main]

nun::entry!(main);

fn main(init_info: &nun::InitInfo) {
    nun::println!("A9N user payload");
    nun::println!(
        "kernel version: {}.{}.{}",
        init_info.kernel_major_version,
        init_info.kernel_minor_version,
        init_info.kernel_patch_version,
    );

    loop {}
}
```

最初のUser Payloadでは，Nunが再公開する`InitInfo`とMacroだけを利用する．Kernel Call Wrapperを直接扱う場合は`a9n_abi`，共有型だけを扱うLibraryは`a9n_types`を用いる．各層の選択基準と責務は「Ecosystem」に記載する．

== Troubleshooting

#reference_table(
  (1.8fr, 2.9fr),
  ([現象], [確認箇所]),
  [CargoまたはLLVMのDynamic Library Error], [`command -v cargo`を確認し，rustupが管理するCargoを選択する．],
  [Submodule内のFileが見つからない], [`git submodule status`を確認し，未取得なら`git submodule update --init --recursive`を実行する．],
  [QEMUがTCP Port 1234を確保できない], [Hostの`127.0.0.1:1234`を使用中のProcessを停止してから再実行する．],
  [`core`をLinkできない], [既定のNun Target JSONを用い，`__init_info_start`と`__init_ipc_buffer_start`を保持する．],
)
