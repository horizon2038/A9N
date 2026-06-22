# A9N Microkernel Kernel Call仕様（実装準拠）

本書は`src/`の現行実装を基準としたKernel Call仕様書である．
対象は`x86_64`実装であり，参照元は主に以下である．

- `src/hal/x86_64/systemcall/_syscall.s`
- `src/hal/x86_64/process/process_manager.cpp`
- `src/kernel/kernelcall/kernel_call.cpp`
- `src/kernel/include/kernel/capability/*.hpp`
- `src/kernel/capability/*.cpp`

## 1. Kernel Callエントリ

### 1.1 Kernel Call種別

`kernel_call_type`は負値で定義される．

- `-1` : `CAPABILITY_CALL`
- `-2` : `YIELD`
- `-3` : `DEBUG`（デバッグ用）

不正なcall番号は`fault_type::INVALID_KERNEL_CALL`としてfault dispatcherへ渡される．

### 1.2 Message RegisterとIPC BufferはArchitecture依存

Kernelは`a9n::hal::get_message_register()`/`configure_message_register()`を通してMRへアクセスする．
この実体はArchitecture実装で規定されるため，MRと実レジスタの対応，およびIPC Bufferへのフォールバック方式はArchitecture依存である．

### 1.3 x86_64実装でのMR対応

`x86_64`では`src/hal/x86_64/process/process_manager.cpp`で以下の対応を取る．

- `RAX` : Kernel Call種別
- `RDI` : `MR0`
- `RSI` : `MR1`
- `RDX` : `MR2`
- `R8`  : `MR3`
- `R9`  : `MR4`
- `R10` : `MR5`
- `R12` : `MR6`
- `R13` : `MR7`
- `R14` : `MR8`
- `R15` : `MR9`

`MR10`以降は`process.buffer->messages[index]`へアクセスする．
このとき`index`はMR番号そのものを使う．

### 1.4 C APIの引数名と実際の意味

`src/api/c/include/api/c/arch/x86_64/kernel_call.h`の`a9n_kernel_call()`は
`(descriptor, depth, tag, message_length)`という引数名を持つが，カーネル側は以下として解釈する．

- `descriptor`引数（RDI）: 実際には`MR0`そのもの
- `depth`引数（RSI）: 実際には`MR1`そのもの
- `tag`引数（RDX）: 実際には`MR2`そのもの
- `message_length`引数（RCX経由R8）: 実際には`MR3`そのもの

つまりCapability Callでは，ユーザー側で`MR0`にraw descriptor，`MR1`にoperation typeを直接構築する必要がある．

## 2. Capability Call共通仕様

### 2.1 共通入力と共通出力

共通入力は以下である．

- `MR0` : Capability Descriptor（raw）
- `MR1` : operation type
- `MR2..` : operation固有引数

共通出力は以下である．

- 成功時 : `MR0 = 1`
- 失敗時 : `MR0 = 0`，`MR1 = capability_error`

`capability_error`の数値は次である．

- `0` : `ILLEGAL_OPERATION`
- `1` : `PERMISSION_DENIED`
- `2` : `INVALID_DESCRIPTOR`
- `3` : `INVALID_DEPTH`
- `4` : `INVALID_ARGUMENT`
- `5` : `FATAL`
- `6` : `DEBUG_UNIMPLEMENTED`

### 2.2 Capability Descriptorの構造

`MR0`の上位8bitは深さ情報であり，残り下位bitは探索対象descriptorである．

```text
+----------------------+--------------------------------------------+
| bit 63 .. 56         | bit 55 .. 0                                |
+----------------------+--------------------------------------------+
| encoded_depth        | descriptor payload                         |
+----------------------+--------------------------------------------+
```

深さ復元は`extract_depth()`で以下となる．

```cpp
depth = ((raw_descriptor >> (WORD_BITS - BYTE_BITS)) & 0xFF) + BYTE_BITS;
```

`WORD_BITS=64`環境では`depth`の取り得る値は`8..263`である．
カーネルはこの値に対する上限検証を行わない．

### 2.3 Addressing機構（`capability_node::traverse_slot`）

探索は`traverse_slot(descriptor, descriptor_max_bits, descriptor_used_bits)`で実行される．
`CAPABILITY_CALL`では初期値として以下が渡される．

- `descriptor = MR0(raw descriptor)`
- `descriptor_max_bits = extract_depth(MR0)`
- `descriptor_used_bits = 8`

Nodeでのindex計算式は以下である．

```cpp
mask_bits  = (1 << radix_bits) - 1;
shift_bits = WORD_BITS - (ignore_bits + radix_bits + descriptor_used_bits);
index      = (descriptor >> shift_bits) & mask_bits;
```

次段へ進む際の`descriptor_used_bits`更新は以下である．

```cpp
new_used_bits = descriptor_used_bits + ignore_bits + radix_bits;
```

`new_used_bits == descriptor_max_bits`になった時点のslotが最終探索結果である．

### 2.4 現行実装におけるAddressing上の実務ポイント

- 初期root nodeおよび`GENERIC->NODE`で生成されるnodeは`ignore_bits=0`で作られる．
- よって通常は「上位から`radix_bits`ずつ食う可変radix木」として扱えばよい．
- `capability_node::retrieve_slot()`はindex境界検証を実装していない．
- `depth`不正時は多くの場合`INVALID_DESCRIPTOR`系で失敗し，明示的に`INVALID_DEPTH`へはマップされない．

### 2.5 IPC Buffer利用条件とPCBへのattach

`x86_64`実装では`MR0..MR9`はレジスタ，`MR10..`はIPC Bufferを使うため，`MR10`以上を使うcallではIPC Bufferが必須である．

最低条件は以下である．

- 対象processの`process.buffer`が有効な`ipc_buffer*`を指すこと
- 対象processの`process.buffer_frame.type == FRAME`であること（Capability Transfer時に必須）

`PROCESS_CONTROL_BLOCK::CONFIGURE`でIPC Buffer用frameをattachする手順は以下である．

1. `configuration_info`のbit2（`frame_ipc_buffer`）を1にする
2. `MR5`へIPC Bufferとして使う`FRAME` capability descriptorを設定する
3. `PROCESS_CONTROL_BLOCK::CONFIGURE`を呼ぶ

この呼び出しで`process_core.buffer_frame`へframe capabilityはコピーされる．
ただし現行実装では，この経路だけでは`process_core.buffer`ポインタは自動設定されない．
`init`生成時のように，kernel側で`ipc_buffer`仮想アドレスを`process_core.buffer`へ設定する追加処理が必要である．

`ipc_buffer`自体の利用面は以下である．

- `messages[]` : `MR10`以上の実データ領域
- `transfer_source_descriptors[]` : 送信側が移譲するcapability descriptor列
- `transfer_destination_node` : 受信側の移譲先node descriptor
- `transfer_destination_index` : 受信側の移譲先開始index

Capability Transferを伴うIPCの最小手順は以下である．

1. 送信側は`transfer_source_descriptors[i]`へ移譲元descriptorを設定する
2. 受信側は`transfer_destination_node`と`transfer_destination_index`を設定する
3. 送信時の`message_info.transfer_count`へ移譲個数を設定する
4. `CALL`または`SEND`/`RECEIVE`の同期でカーネルが`try_move_capability_slot()`を実行する

## 3. Capability Slotと権限

`capability_slot`の権限ビットは以下である．

- `READ = 1 << 0`
- `WRITE = 1 << 1`
- `COPY = 1 << 2`
- `MODIFY = 1 << 3`

`ALL = READ | WRITE | COPY | MODIFY`である．

Capability typeは以下である．

- `NONE`
- `DEBUG`
- `NODE`
- `GENERIC`
- `ADDRESS_SPACE`
- `PAGE_TABLE`
- `FRAME`
- `PROCESS_CONTROL_BLOCK`
- `IPC_PORT`
- `NOTIFICATION_PORT`
- `INTERRUPT_REGION`
- `INTERRUPT_PORT`
- `IO_PORT`
- `VIRTUAL_CPU`
- `VIRTUAL_ADDRESS_SPACE`
- `VIRTUAL_PAGE_TABLE`

## 4. Capability別 Capability Call仕様

以下の`MRn`はすべてCapability Call文脈でのMRを指す．

### 4.1 NODE (`capability_node`)

operation type:

- `1` : `COPY`
- `2` : `MOVE`
- `3` : `MINT`
- `4` : `DEMOTE`
- `5` : `REVOKE`
- `6` : `REMOVE`

共通前提:

- `MR0` : このnode slotを指すraw descriptor
- `MR1` : operation type

#### COPY

- `MR2` : destination index（このnode配下）
- `MR3` : source descriptor（rootから探索）

検証:

- 呼び出し元node slotに`READ|WRITE`必須
- source slotに`COPY`必須
- destination slotは空である必要がある（`try_copy_capability_slot`）

動作:

- source capabilityを複製し，dependency node上はsiblingとして接続する．

#### MOVE

- `MR2` : destination index
- `MR3` : source descriptor

検証:

- 呼び出し元node slotに`READ|WRITE`必須
- destination slotは空である必要がある

動作:

- source slotの実体をdestinationへ移動し，sourceを初期化する．

#### MINT

- `MR2` : destination index
- `MR3` : source descriptor
- `MR4` : new rights

検証:

- 呼び出し元node slotに`READ|WRITE`必須
- source slotに`COPY`必須
- `new_rights`が`source_rights`の部分集合であること

動作:

- COPY後にdestinationのrightsを`new_rights`へ置換する．

#### DEMOTE

- `MR2` : target index（このnode配下）
- `MR4` : new rights

検証:

- 呼び出し元node slotに`WRITE`必須
- target slotに`READ|WRITE`必須
- `new_rights`がtarget rightsの部分集合であること

動作:

- target slotのrightsを`new_rights`へ縮退する．

#### REVOKE

- `MR2` : target index（このnode配下）

検証:

- target slotに`READ|WRITE`必須

動作:

- targetがself componentと同一なら`revoke(self)`を呼ぶ．
- それ以外はchild削除を試みた後，`slot->component->revoke(*slot)`を呼ぶ．

実装注意:

- child削除ループ条件が`start_slot->depth < slot->depth`であり，通常の子孫（深さ増加）には一致しないため，意図した再帰削除にならない可能性がある．

#### REMOVE

- `MR2` : target index（このnode配下）

検証:

- 呼び出し元node slotの事前検証は`!(READ) && !(WRITE)`であり，READのみ/WRITEのみでも通過する．
- 実際の削除対象slot側では`READ|WRITE`両方を要求する．

動作:

- `target_slot->try_remove_and_init()`を実行する．

### 4.2 GENERIC (`generic`)

operation type:

- `0` : `CONVERT`

#### CONVERT

- `MR2` : `capability_type`
- `MR3` : `specific_bits`
- `MR4` : `count`
- `MR5` : `root_descriptor`（配置先node descriptor）
- `MR6` : `slot_index`（配置開始index）

検証:

- `count==0`は`INVALID_ARGUMENT`
- device generic（`is_device==true`）は`FRAME`以外へ変換不可
- 対象slotは空である必要がある
- genericの残容量不足は`ILLEGAL_OPERATION`

`self.data`（generic slot data）:

- `data[0]` : base physical address
- `data[1]` : flags（bit7=device，bit0..6=size_bits）
- `data[2]` : watermark

`try_make_capability()`での型別挙動:

- `NODE` : `specific_bits`をradixとしてnode生成
- `GENERIC` : `specific_bits`をchild generic size_bitsとして生成
- `ADDRESS_SPACE` : root page tableを生成
- `PAGE_TABLE` : page tableを生成し，flagsへ`depth=specific_bits`を保持
- `FRAME` : `specific_bits`はframe size bitsとして検証・使用
- `PROCESS_CONTROL_BLOCK` : PCB実体を生成
- `IPC_PORT` : IPC port実体を生成（identifier初期値0）
- `NOTIFICATION_PORT` : notification port実体を生成（identifier初期値0）
- `IO_PORT` : IO port capability実体を生成（rangeは全域）
- `VIRTUAL_CPU` : vCPU capability実体を生成
- `DEBUG`/`INTERRUPT_REGION`/`INTERRUPT_PORT`/`VIRTUAL_PAGE_TABLE`/その他 : 未実装

### 4.3 ADDRESS_SPACE (`address_space`)

operation type:

- `1` : `MAP`
- `2` : `UNMAP`
- `3` : `GET_UNSET_DEPTH`
- `4` : `CAN_MAP_FRAME_SIZE_BITS`

#### MAP

- `MR2` : map descriptor（`PAGE_TABLE`または`FRAME`）
- `MR3` : map address（user address）
- `MR4` : attribute（現実装未使用）

動作:

- self slotのpage tableをrootとして，対象descriptorのpage table/frameを`map_*`する．

#### UNMAP

- `MR2` : unmap descriptor（`PAGE_TABLE`または`FRAME`）
- `MR3` : unmap address

動作:

- `unmap_page_table`または`unmap_frame`を呼ぶ．

#### GET_UNSET_DEPTH

- `MR2` : target virtual address
- `MR3` : leaf size bits
- 戻り値 `MR2` : unset depth

#### CAN_MAP_FRAME_SIZE_BITS

- `MR2` : frame size bits
- 追加戻り値なし（成功/失敗のみ）

実装注意:

- `ADDRESS_SPACE`操作ではself rightsを明示チェックしていない．

### 4.4 PAGE_TABLE (`page_table_capability`)

- `execute()`は常に`ILLEGAL_OPERATION`を返す．

### 4.5 FRAME (`frame_capability`)

定義上operation type:

- `1` : `GET_ADDRESS`

`GET_ADDRESS`定義上の戻り値:

- `MR2` : frame physical address

実装現状:

- `execute()`自体が常に`DEBUG_UNIMPLEMENTED`を返し，`GET_ADDRESS`へ到達しない．

### 4.6 PROCESS_CONTROL_BLOCK (`process_control_block`)

operation type:

- `1` : `CONFIGURE`
- `2` : `READ_REGISTER`
- `3` : `WRITE_REGISTER`
- `4` : `RESUME`
- `5` : `SUSPEND`

#### CONFIGURE

- `MR2` : `configuration_info`ビットマスク
- `MR3` : address space descriptor
- `MR4` : root node descriptor
- `MR5` : frame ipc buffer descriptor
- `MR6` : notification port descriptor
- `MR7` : ipc port resolver descriptor
- `MR8` : instruction pointer
- `MR9` : stack pointer
- `MR10`: thread local base
- `MR11`: priority
- `MR12`: affinity

`configuration_info`ビット:

- bit0 : address_space
- bit1 : root_node
- bit2 : frame_ipc_buffer
- bit3 : notification_port
- bit4 : ipc_port_resolver
- bit5 : instruction_pointer
- bit6 : stack_pointer
- bit7 : thread_local_base
- bit8 : priority
- bit9 : affinity

補足:

- `notification_port`設定は`DEBUG_UNIMPLEMENTED`
- `instruction_pointer`/`stack_pointer`/`thread_local_base`はuser address検証あり
- `priority`は`priority < PRIORITY_MAX`検証あり

#### READ_REGISTER

- `MR2` : register_count
- 戻り値 : `MR3..`へ`process_core.registers[]`を`register_count`個書き戻す

#### WRITE_REGISTER

- `MR2` : register_count
- `MR3..` : 書き込み値

#### RESUME

- 対象processを`READY`化し，schedulerへ登録する．

#### SUSPEND

- 対象processを`BLOCKED`化し，priorityを0へ設定する．

### 4.7 IPC_PORT (`ipc_port`)

operation type:

- `1` : `SEND`
- `2` : `RECEIVE`
- `3` : `CALL`
- `4` : `REPLY`
- `5` : `REPLY_RECEIVE`
- `6` : `IDENTIFY`

MRレイアウト:

- `MR2` : `message_info`（`IDENTIFY`時はidentifier入力として使用）
- `MR3` : identifier destination
- `MR4..` : payload

`message_info`ビット:

- bit0 : block
- bit1..8 : message_length
- bit9..14 : transfer_count
- bit15 : kernel

権限:

- `SEND`/`CALL` : selfに`WRITE`必須
- `RECEIVE` : selfに`READ`必須
- `REPLY`/`REPLY_RECEIVE` : rights不要
- `IDENTIFY` : selfに`MODIFY`必須

Capability Transfer:

- `transfer_count > 0`時，sender/receiver双方で`process.buffer`と`buffer_frame.type==FRAME`が必要
- 受信先nodeは`destination_process.buffer->transfer_destination_node/index`
- 送信元descriptor列は`source_process.buffer->transfer_source_descriptors[]`

ブロッキング:

- `message_info.block==0`なら待機せず即returnする分岐を持つ

fault連携:

- resolver port経由のfault配送では，`kernel`ビット付きmessageをカーネルが構築する
- fault messageは`MR0..`に`fault.hpp`定義のフォーマットで書かれる

### 4.8 NOTIFICATION_PORT (`notification_port`)

operation type:

- `1` : `NOTIFY`
- `2` : `WAIT`
- `3` : `POLL`
- `4` : `IDENTIFY`

#### NOTIFY

- slot local identifierをnotification bitとして立てる
- waiterがいれば1件起床し，`MR2`へconsume結果を書いて直接スケジュール切替する

#### WAIT

- 通知があれば即consumeして`MR2`へ返す
- 通知がなければ呼び出し元processをblockし，wait queueへ入れる

#### POLL

- 非ブロッキング
- 通知があれば`MR2`へ返し，なければ成功扱いで何も返さない

#### IDENTIFY

- `MR2` : new identifier
- `MODIFY`権限必須

### 4.9 INTERRUPT_REGION (`interrupt_region`)

operation type:

- `1` : `MAKE_PORT`

`MAKE_PORT`引数:

- `MR2` : irq number
- `MR3` : target node descriptor
- `MR4` : target node index

動作:

- target slotが空であることを確認し，`INTERRUPT_PORT` capabilityを生成する
- 同一IRQが既に`used`なら`ILLEGAL_OPERATION`

### 4.10 INTERRUPT_PORT (`interrupt_port`)

operation type:

- `1` : `BIND_NOTIFICATION_PORT`
- `2` : `UNBIND_NOTIFICATION_PORT`
- `3` : `ACK`
- `4` : `GET_IRQ_NUMBER`

#### BIND_NOTIFICATION_PORT

- `MR2` : notification port descriptor

動作:

- descriptorが`NOTIFICATION_PORT`であることを確認
- IRQ handler slotへcopyしてbindする

#### UNBIND_NOTIFICATION_PORT

- 引数なし
- bind済みslotを`try_remove_and_init()`する

#### ACK

- 引数なし
- 対応IRQを`interrupt_manager.enable_interrupt()`で再有効化する

#### GET_IRQ_NUMBER

- 戻り値 `MR2` : irq number

### 4.11 IO_PORT (`io_port_capability`)

operation type:

- `1` : `READ`
- `2` : `WRITE`
- `3` : `MINT`

#### READ

- `MR2` : source port
- `MR3` : read width
- 戻り値 `MR2` : read data

権限:

- selfに`READ`必須

備考:

- 実装コメント上，READパスのrange検証はTODOである

#### WRITE

- `MR2` : destination port
- `MR3` : write width
- `MR4` : write data

権限:

- selfに`WRITE`必須
- slot dataのport range内であること

#### MINT

- `MR2` : new range min
- `MR3` : new range max
- `MR4` : destination node descriptor
- `MR5` : destination node index

検証:

- new rangeがself rangeの部分集合
- destination slotは空

動作:

- destinationへ新しい`IO_PORT` capabilityを構成する

### 4.12 VIRTUAL_CPU (`virtual_cpu_capability`)

定義上operation type:

- `1` : `CONFIGURE_ADDRESS_SPACE`
- `2` : `CONFIGURE_STATE_DESCRIPTOR`
- `3` : `READ_STATE`
- `4` : `WRITE_STATE`
- `5` : `ENTER`
- `6` : `EXIT`（予約）
- `7` : `INJECT_IRQ`

実装現状:

- `execute()`が`MR1`を読まず`type=0`固定で分岐している
- 結果として常に`ILLEGAL_OPERATION`となる

## 5. fault messageフォーマット（IPC resolver向け）

fault配送時，カーネルは以下を設定する．

共通ヘッダ:

- `MR0` : is_success（1）
- `MR1` : error_code（0）
- `MR2` : message_info（kernel bit付き）
- `MR3` : identifier
- `MR4` : `fault_type`

種別別ペイロード:

- `MEMORY`/`MEMORY_INSTRUCTION_FETCH` : `MR5=PC`，`MR6=fault_address`，`MR7=arch_fault_code`
- `INVALID_INSTRUCTION` : `MR5=PC`，`MR6=arch_fault_code`
- `INVALID_ARITHMETIC` : `MR5=PC`，`MR6=arch_fault_code`
- `INVALID_KERNEL_CALL` : `MR5=PC`，`MR6=kernel_call_number`
- `ARCHITECTURE` : `MR5=PC`，`MR6=arch_fault_code`

## 6. 開発時の注意点

- Descriptor depthの上限検証はないため，ユーザー側で妥当値（通常64bit以下）を保証する必要がある．
- Node slot取得は境界チェック未実装のため，index管理をユーザー側で厳密に行う必要がある．
- `MR10`以降を使う操作はIPC bufferの有効な設定が前提である．
- `PAGE_TABLE`，`FRAME`，`VIRTUAL_CPU`は実質スタブ扱いの経路が残っている．
- `a9n_kernel_call()`の引数名は実ABIと一致していないため，MR意味で扱う必要がある．

## 7. 最小呼び出しテンプレート

```text
RAX = -1 (CAPABILITY_CALL)
MR0 = raw_descriptor
MR1 = operation_type
MR2.. = arguments
syscall
if MR0 == 1 then success else error = MR1
```

raw descriptor組み立ては以下である．

```text
encoded_depth = (depth_bits - 8) & 0xFF
raw_descriptor = (encoded_depth << 56) | descriptor_payload
```

`depth_bits`は`traverse`完了時に`descriptor_used_bits`と一致する値を設定する必要がある．
通常は「8（depth field）+ 各nodeの(ignore_bits + radix_bits)合計」を使う．
