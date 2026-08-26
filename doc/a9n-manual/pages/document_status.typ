#import "/components/reference.typ" : reference_table, term

#let document_status(version) = [
  = Document Status

  本書は，#term[A9N Microkernel]の利用者，User-level Systemの開発者，HALの移植者を対象とするManualである．OS，Process，Virtual Memory，System Call，Interrupt，C，C++，Rustの基礎知識を前提とし，A9N固有の知識は前提としない．

  #reference_table(
    (1.35fr, 3.5fr),
    ([項目], [説明]),
    [Manual Version], [#raw(version.trim())．],
    [実装Architecture], [x86_64．Architectureに依存しないInterfaceを先に説明し，固有のRegister，Page Table，Boot，I/Oを「#term[x86_64 ABI]」にまとめる．],
    [Multiprocessing], [`A9N_CONFIG_ENABLE_SMP`でBuild時に選択する．x86_64は最大64 Coreを起動し，Kernel EntryをGiant Lockで直列化する．],
    [Source of Truth], [#term[SPENCER]がSubmoduleとして取得するA9Nの公開Headerと実装を基準とする．User-level Rust Interfaceは#term[a9n_abi]と#term[a9n_types]を基準とする．],
    [互換性], [Stable ABIの保証期間とBackward Compatibilityの期間は定義されていない．Componentの組合せはSPENCERとUser Payloadの`Cargo.lock`が固定するVersionを基準とする．],
  )

  数値，Type，Operation，Data Layoutは公開Interface定義を優先する．State TransitionとError変換は実装を参照する．Testと既存Markdownは補助資料であり，公開Interfaceまたは実装と競合する説明を仕様として扱わない．
]
