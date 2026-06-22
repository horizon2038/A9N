# Init Guide（Kernel User向け）

本書はA9N上でInit（最初のユーザプロセス）を実装する開発者向けガイドである．
対象はKernel Developerではなく，Kernel User（OS/Application開発者）である．

## 1. 目的

Init実装で最低限必要な以下を示す．

- 起動直後に利用できるデータ
- 起動直後に利用できるCapability
- IPC Bufferの利用前提
- Architecture非依存部分と依存部分の境界

Kernel内部実装の詳細は
[doc/kernel-init-implementation.md](./kernel-init-implementation.md)を参照すること．
Kernel Call ABIは
[doc/kernel-call.md](./kernel-call.md)を参照すること．

## 1.1 ブートストラップ前提

Initは単体で起動されるのではなく，bootloaderとkernelの連携で起動される．
前提は以下である．

1. bootloaderがkernelイメージとinitイメージをメモリへ配置し，kernelへ制御を移す
2. kernelはbootloaderから受け取ったメモリマップとInit配置情報を使い，Init processを構築して起動する

この設計で重要なのは，kernelがELFフォーマット自体を解釈しないことである．
kernelが参照するのは`boot_info`に渡された結果情報であり，イメージ形式の解釈責務はbootloader側にある．

## 2. Architecture非依存の契約

### 2.1 Initが受け取る`init_info`

Initは起動直後に`init_info`を受け取る．
`init_info`には以下が含まれる．

- Kernel version
- `arch_info[ARCH_INFO_MAX]`
- IPC Bufferアドレス（`ipc_buffer`）
- 初期Generic一覧（`generic_list[]`，`generic_list_count`）

`arch_info`は配列であるが，意味付けはArchitecture依存である．
この点は本書の依存セクションで扱う．
`init_info`自体は，bootloaderが指定したInitイメージ内位置に対してkernelが書き込む起動契約領域である．

### 2.2 Init root nodeの初期slot

Initのroot nodeには以下slotが初期配置される．

- `1` : `PROCESS_CONTROL_BLOCK`
- `2` : `PROCESS_ADDRESS_SPACE`
- `3` : `PROCESS_ROOT_NODE`
- `4` : `PROCESS_PAGE_TABLE_NODE`
- `5` : `PROCESS_FRAME_NODE`
- `6` : `PROCESS_IPC_BUFFER_FRAME`
- `7` : `GENERIC_NODE`
- `8` : `INTERRUPT_REGION`

`IO_PORT`のslot番号は定義されるが，初期状態で必ず有効Capabilityが入る保証はない．

### 2.3 IPC Bufferの役割

IPC Bufferは以下を提供する．

- `messages[]` : 高番号MRのバックストア
- `transfer_source_descriptors[]` : Capability Transfer送信元descriptor列
- `transfer_destination_node` : 受信側の転送先node descriptor
- `transfer_destination_index` : 受信側の転送先開始index

Capability Transferを行う場合，送受信側でIPC Buffer内容を事前設定する必要がある．

### 2.4 Init側で最初に実行すること

推奨順序は以下である．

1. `init_info`ポインタを確立する
2. `init_info.ipc_buffer`からIPC Bufferポインタを確立する
3. `generic_list[]`を走査して初期資源を台帳化する
4. `PROCESS_CONTROL_BLOCK`と`GENERIC_NODE`を起点に追加資源を構成する

## 3. Architecture依存の契約

### 3.1 `arch_info[]`の意味はArchitectureごとに異なる

`arch_info[]`は共通配列であり，indexごとの意味はArchitecture実装が定義する．
よって，あるArchitectureで`arch_info[0]`に入る値が，別Architectureでも同じ意味になる保証はない．

例として，`RSDP address`が`arch_info[0]`に入る設計は
`x86_64`実装の選択であり，汎用仕様ではない．

### 3.2 `x86_64`での`arch_info[]`例

現行`x86_64`実装では，`arch_initializer::init_architecture(arch_info[])`が
`arch_info[0]`を`rsdp_address`として`init_main_core()`へ渡す．

したがって`x86_64`では以下が成立する．

- `arch_info[0]` : ACPI RSDPの物理アドレス

ただしこれは`x86_64`固有である．
他Architectureへ移植するInitは，`arch_info[]`解釈レイヤを分離して実装する必要がある．

### 3.3 MRとレジスタ対応はArchitecture依存

Kernelは抽象的に`MRn`を扱うが，`MRn`と実レジスタの対応はArchitecture実装が定義する．

現行`x86_64`では以下である．

- `MR0..MR9` : `RDI, RSI, RDX, R8, R9, R10, R12, R13, R14, R15`
- `MR10..` : `ipc_buffer.messages[index]`

よって，`MR10`以上を使うKernel CallはIPC Bufferが正しく設定されていることを前提とする．

## 4. Init最小ひな型

以下は最小構成例である．
リンクスクリプト上で`init_info`領域とstack領域を公開している前提である．

```c
extern char __init_info_start[];
extern char __init_stack_end[];

struct init_info;
struct ipc_buffer;

void main(void);

void _start(void)
{
    asm volatile("mov %0, %%rsp" : : "r"(__init_stack_end));

    struct init_info *info = (struct init_info *)__init_info_start;
    struct ipc_buffer *ipc = (struct ipc_buffer *)info->ipc_buffer;

    (void)ipc;
    main();
}
```

## 5. Kernel Call開始時の実務ポイント

- 最初は`GENERIC_NODE`から必要Capabilityを`CONVERT`で生成する流れを取る
- descriptor構築は`raw descriptor`形式に従う
- 戻り値は`MR0`成功フラグと`MR1`エラーコードで判定する

operation番号とMRレイアウトは
[doc/kernel-call.md](/Users/horizon/Documents/Program/A9N/doc/kernel-call.md)を正として実装すること．

## 6. 実装上の注意

- `generic_list_count`は利用前に上限チェックを入れる
- `arch_info[]`を固定indexで直接利用する実装は移植性を失う
- `x86_64`依存処理は必ず分離する
- `MR10`以上を使う前にIPC Buffer前提が満たされているか確認する

## 7. 推奨実装方針

1. `boot contract`層 : `init_info`受け取りと`arch_info`解釈
2. `capability bootstrap`層 : root nodeから初期Capability解決
3. `service bootstrap`層 : メモリ管理，IPC，割り込み管理の初期サービス起動

この3層を分離すると，Architecture追加時は`boot contract`層の差し替えで対応しやすくなる．
