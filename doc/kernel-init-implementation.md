# A9N Init Process仕様（実装準拠）

本書は`src/`の現行実装に同期したInit生成仕様である．
対象は`x86_64`実装であり，カーネルが最初のユーザプロセス（Init）をどのように構築するかを定義する．

## 1. 対象範囲

対象範囲は以下である．

- `src/kernel/main.cpp`
- `src/kernel/boot/init.cpp`
- `src/kernel/include/kernel/boot/init.hpp`
- `src/kernel/include/kernel/boot/boot_info.hpp`
- `src/kernel/include/kernel/ipc/ipc_buffer.hpp`
- `src/servers/arch/x86_64/user.ld`

本書は実装仕様書であり，理想設計ではなく現行コードの振る舞いを記述する．

## 2. ブートからInit起動までの経路

`kernel_entry()`は以下の順で初期化を行う．

1. HAL初期化
2. 割り込み系初期化
3. process manager初期化
4. `create_init(boot_info)`呼び出し
5. `process_manager_core.switch_to_user()`でInitへ遷移

したがってInit生成の主処理は`create_init()`に集約される．

## 3. 入力データ構造

### 3.1 `boot_info`

`boot_info`は以下の3要素から構成される．

- `boot_memory_info` : メモリマップ
- `boot_init_image_info` : Initイメージ情報
- `arch_info[ARCH_INFO_MAX]` : アーキテクチャ依存情報

### 3.2 `init_image_info`

`init_image_info`は以下を持つ．

- `loaded_address` : Initイメージ物理ロード先
- `init_image_size` : Initイメージサイズ情報
- `entry_point_address` : Initエントリ仮想アドレス
- `init_info_address` : Initイメージ内`init_info`の仮想オフセット
- `init_ipc_buffer_address` : Initイメージ内IPC Bufferの仮想オフセット

これらの値はbootloaderが`boot_info`へ設定してカーネルへ渡す入力値である．
カーネルは妥当性を包括検証せず，この値群を前提としてInitを構成する．
したがってbootloader側で`loaded_address`，`entry_point_address`，`init_info_address`，`init_ipc_buffer_address`の整合を保証する必要がある．

実装上，`init_image_size`は`try_configure_init_frames()`でフレーム個数として扱われる．
変数名はsizeであるが，処理は`for (i = 0; i < init_image_size; i++)`である．

## 4. `init_info`仕様

### 4.1 構造

`init_info`は以下のフィールドを持つ．

- `kernel_major_version`
- `kernel_minor_version`
- `kernel_patch_version`
- `kernel_pre_release[32]`
- `kernel_build_meta_data[32]`
- `arch_info[ARCH_INFO_MAX]`
- `ipc_buffer`
- `generic_list[INITIAL_GENERIC_COUNT_MAX]`
- `generic_list_count`

`sizeof(init_info) <= PAGE_SIZE`がstatic_assertされる．

### 4.2 配置と参照

カーネルは`init_info`の実体を以下で取得する．

- `init_info_physical = loaded_address + init_info_address`
- `physical_to_virtual_pointer<init_info>(init_info_physical)`

このため，`init_info_address`は実装上「Initイメージ先頭からの相対位置」として使われる．

### 4.3 フィールドの埋め方

`try_configure_init_info()`で以下を設定する．

- バージョン情報 : `KERNEL_VERSION_STRING`をsemantic versionとして分解
- `arch_info[]` : `boot_info.arch_info`をコピー
- `ipc_buffer` : `boot_init_image_info.init_ipc_buffer_address`をそのまま設定
- `generic_list[]` : メモリマップから生成

## 5. Initイメージ側の要求（`user.ld`）

`src/servers/arch/x86_64/user.ld`は`.bss`内に以下の予約領域を作る．

- `__init_info_start`〜`__init_info_end` : `0x1000 * 2`
- `__init_ipc_buffer_start`〜`__init_ipc_buffer_end` : `0x1000`
- `__init_stack_start`〜`__init_stack_end` : `0x1000`

カーネルはこれらシンボル名を直接参照しないが，ブートローダが
`init_info_address`と`init_ipc_buffer_address`を正しく設定する前提となる．

## 6. `create_init()`の処理順序

`create_init()`は2段構成である．

1. `create`フェーズ : 構造体と空Capability木を生成
2. `configure`フェーズ : 中身を設定し，実行可能なprocessへ仕上げる

## 7. createフェーズ詳細

### 7.1 PCB slotとPCB実体の確保

- `init_allocator`（`PAGE_SIZE * 512`のlinear allocator）から確保する
- PCB slotに`try_configure_process_control_block_slot()`を適用する

### 7.2 Init processのnode構築

`try_create_init_process_nodes()`で以下を作成する．

- ルートnode（`INITIAL_PROCESS_ROOT_NODE_COUNT = 256`）
- page table node（`INITIAL_PAGE_TABLE_COUNT_MAX = 128`）
- frame node（`INITIAL_FRAME_COUNT_MAX = 16384`）
- generic node（`INITIAL_GENERIC_COUNT_MAX = 128`）

各nodeは`try_make_node()`経由で以下手順を取る．

1. slot配列を確保
2. `capability_node(ignore_bits=0, radix=size_radix, slots=...)`を配置new
3. 対象slotへ`try_configure_capability_node_slot()`

### 7.3 ルートnodeへの初期参照コピー

`init_slot_offset`に従い以下を配置する．

- `PROCESS_CONTROL_BLOCK` : Init自身のPCB capability
- `PROCESS_ROOT_NODE` : Init自身のroot node capability（再帰参照）

## 8. configureフェーズ詳細

### 8.1 レジスタとメタデータ

`try_configure_init_process_control_block()`で以下を設定する．

- `hal::init_hardware_context(USER, registers)`
- `INSTRUCTION_POINTER = entry_point_address`
- `process name = "INIT"`

その後，address space系を構成し，`mark_scheduled()`でREADY登録する．

注意点として，この経路ではstack pointerを明示設定しない．

### 8.2 ルートAddress Space構成

`try_configure_init_root_address_space()`で以下を行う．

1. 1ページ分メモリを確保
2. `hal::make_address_space()`でroot page tableを生成
3. `process_core.root_address_space`へ設定
4. Init root nodeの`PROCESS_ADDRESS_SPACE`スロットへcopy

### 8.3 Page Tableノード構成

`try_configure_init_page_tables()`は以下ロジックで不足page tableを作る．

- `root_table`を基準に`search_unset_page_table_depth()`を呼ぶ
- 深さが必要な位置に新規page tableを確保して`map_page_table()`する
- 作成したpage table capabilityを`PROCESS_PAGE_TABLE_NODE`へ順次格納する

仮想範囲は`map_address = 0`から`last_mapped_virtual_address`まで走査する．
`last_mapped_virtual_address = init_image_size * FRAME_SIZE`である．

### 8.4 Frameノード構成とInitイメージmap

`try_configure_init_frames()`は`i = 0..init_image_size-1`について以下を行う．

- `physical = loaded_address + FRAME_SIZE * i`
- `virtual = FRAME_SIZE * i`
- `map_frame(root_table, frame, virtual)`
- frame capabilityを`PROCESS_FRAME_NODE[i]`へ格納

これによりInitイメージ全体を`0x0`起点で連続mapする．

### 8.5 IPC Buffer frameの特別処理

`frame.address == loaded_address + init_ipc_buffer_address`のframeに一致したとき，
以下の追加設定を実行する．

- `process_core.buffer = physical_to_virtual_pointer<ipc_buffer>(frame_ipc_buffer_base)`
- `PROCESS_IPC_BUFFER_FRAME`スロットへ同frameを設定
- `process_core.buffer_frame`へ同frameを設定
- dependency node上でsiblingとして接続

この結果，Init processは以下2条件を満たす．

- `process.buffer`が有効
- `process.buffer_frame.type == FRAME`

これはIPCおよびCapability Transfer実行の前提である．

### 8.6 Genericノード構成

`try_configure_init_generics()`は`init_info.generic_list_count`件を走査し，
`GENERIC_NODE[i]`に`GENERIC` capabilityを設定する．

各slot dataは以下で初期化される．

- base = `generic_descriptor.address`
- size_bits = `generic_descriptor.size_radix`
- watermark = base
- is_device = `generic_descriptor.is_device`

### 8.7 Interrupt Region構成

`try_configure_init_interrupt_region()`で`INTERRUPT_REGION`スロットへ
`try_configure_interrupt_region_slot()`を適用する．

### 8.8 IO_PORTスロット

`init_slot_offset`には`IO_PORT`が定義されるが，`create_init()`では未構成である．
初期状態で有効IO_PORT capabilityは配られない．

## 9. `init_slot_offset`一覧

初期root nodeで使用されるslot番号は以下である．

- `0` : `RESERVED`
- `1` : `PROCESS_CONTROL_BLOCK`
- `2` : `PROCESS_ADDRESS_SPACE`
- `3` : `PROCESS_ROOT_NODE`
- `4` : `PROCESS_PAGE_TABLE_NODE`
- `5` : `PROCESS_FRAME_NODE`
- `6` : `PROCESS_IPC_BUFFER_FRAME`
- `7` : `GENERIC_NODE`
- `8` : `INTERRUPT_REGION`
- `9` : `IO_PORT`

## 10. Generic descriptor生成アルゴリズム

`try_configure_init_generic_descriptors()`はmemory mapを走査し，`RESERVED`以外を処理する．

1エントリごとに以下を行う．

1. `size_radix = floor(log2(page_count * PAGE_SIZE))`
2. `[start, start + 2^size_radix)`を1個のgeneric descriptorとして確定
3. 残差領域をページ境界で切り詰め，最大7回まで再帰分割

注意点:

- `memory_map_count > INITIAL_GENERIC_COUNT_MAX`で即失敗する
- 再帰分割で増えた件数について`generic_list_count`更新は外側ループでのみ行われる
- そのため，`generic_list_count`と実際に書き込まれた要素数が一致しない場合がある

## 11. IPC BufferとArchitecture依存性

IPC Bufferの意味付けはArchitecture非依存のデータ構造であるが，
MRと実レジスタの対応はArchitecture実装で規定される．

`x86_64`では以下である．

- `MR0..MR9` : `RDI, RSI, RDX, R8, R9, R10, R12, R13, R14, R15`
- `MR10..` : `process.buffer->messages[index]`

よって，`MR10`以上を使うkernel callはIPC Bufferの正しいattachが必須である．

## 12. Init Server実装時の最小チェックリスト

Initサーバ側は起動直後に以下を満たす必要がある．

1. `init_info`を`__init_info_start`から読み取れること
2. `init_info.ipc_buffer`を`ipc_buffer*`として扱えること
3. `init_slot_offset`の初期capability配置を前提に動作すること
4. `GENERIC_NODE`と`generic_list_count`の不一致可能性を考慮すること
5. `PROCESS_CONTROL_BLOCK` capabilityで追加設定が必要なら明示実行すること

## 13. 実装依存の注意点

- `init_image_size`は名称に反してフレーム個数として使われる実装である．
- stack pointerは`create_init()`で設定されない．
- IO_PORTは初期配布されない．
- `generic_list_count`更新は再帰分割分を反映しない場合がある．
- `init_allocator`は単調増加であり，解放は行わない．
