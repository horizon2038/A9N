#import "/components/reference.typ" : reference_table, fields, notice, term

= Virtualization Status

== Status

#term[Virtualization Capability]群はExperimental Stubである．Type値とVirtual CPU Objectは存在するが，ユーザ空間からVirtual Machineを構成し，Guestを実行する経路は完成していない．

#reference_table(
  (1.4fr, 0.8fr, 1.2fr, 2.3fr),
  ([*Object*], [*Type*], [*Creation*], [*Capability Call*]),
  [`VIRTUAL_CPU`], [`13`], [Genericの`CONVERT`で作成できる．], [`execute()`はOperation Registerを読まず，Type 0固定で`ILLEGAL_OPERATION`を返す．],
  [`VIRTUAL_ADDRESS_SPACE`], [`14`], [作成経路なし．], [Component実装なし．],
  [`VIRTUAL_PAGE_TABLE`], [`15`], [Generic変換のSize計算で拒否される．], [Component実装なし．],
)

== Declared Virtual CPU Operations

Virtual CPU Headerは次のOperation Numberを宣言する．宣言は利用可能性を保証しない．対象Revisionでは全OperationがDispatchされない．

#reference_table(
  (0.8fr, 2fr, 2.5fr),
  ([*Number*], [*Operation*], [*Declared input or result*]),
  [`1`], [`CONFIGURE_ADDRESS_SPACE`], [`MR2 = virtual address space descriptor`．],
  [`2`], [`CONFIGURE_STATE_DESCRIPTOR`], [`MR2 = state descriptor`，`MR3 = exit reason`．],
  [`3`], [`READ_STATE`], [`MR2`以降へStateを返す想定．],
  [`4`], [`WRITE_STATE`], [`MR2 = state descriptor`，`MR3`以降がState．],
  [`5`], [`ENTER`], [`MR2 = state descriptor`，Resultは`MR2`と`MR3`を使用する想定．],
  [`6`], [`EXIT`], [Reserved．],
  [`7`], [`INJECT_IRQ`], [`MR2 = virtual IRQ number`．],
)

`configure_address_space()`，`configure_state_descriptor()`，`read_state()`，`write_state()`，`enter()`，`inject_irq()`は，処理を行わずに成功を返す．`execute()`から呼ばれないため，公開された操作として使用してはならない．

アーキテクチャ固有の補助実装も，Virtual CPUのCapability Callには接続されていない．対象Revisionに含まれる実装の状態は，アーキテクチャごとのABIに記載する．

#notice(
  [WARNING],
  [Virtualization APIを隔離境界として使用してはならない．Guest Memoryの隔離，Host Stateの復元，VM Exit処理，Reserved Bitの検査は実装されていない．対象Revisionでは，Virtualization Capability群を利用できない．],
)

== Completion Requirements

Virtualizationを利用可能にするには，少なくとも次のInterfaceと実装が必要である．

- Virtual Address SpaceとVirtual Page Tableの作成，Mapping，Invalidation．
- Virtual CPU Operation NumberのDecodeとRights検査．
- Architecture-dependent State Descriptor Layout．
- Guest Entry前のPreconditionとGuest Exit後のPostcondition．
- Exit Reason，Register State，Control State，Reserved BitのABI．
- Virtual IRQ InjectionとPending Semantics．
- CPU Affinity，Concurrent Call，Migrationの規則．
- Revoke，Destroy，Architecture固有の制御Memoryを再利用する規則．
- 失敗時のHost Context RestoreとError Mapping．
- Hardware支援機能がないCPUでのCompatibility Result．

対象Revisionでは，Type値13から15と宣言済みOperation Numberを利用可能なABIとして扱ってはならない．ユーザ空間からの呼出しは禁止する．
