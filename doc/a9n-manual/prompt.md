# 実行パラメータ

下記のパラメータに従ってレビューせよ．
利用者が原則として変更する範囲は，本項目だけである．

```text
TARGET_DOCUMENT_NAME = A9N Microkernel Manual
DOCUMENT_FAMILY = MANUAL

TARGET_SECTION_NAME = Overview
TARGET_SECTION_TYPE = OVERVIEW
TARGET_SECTION_LOCATOR = 「Overview」，「概要」，「A9N Microkernel」のいずれかに該当する節

TARGET_AUDIENCE_ROLE = OS・Systems分野の開発者
TARGET_AUDIENCE_BASELINE = CSの基礎知識を持つが，A9N固有の知識を持たない読者
TARGET_READER_GOAL = 対象セクションが説明する概念，仕様または操作を正確に理解し，必要な判断または実装を行う

OUTPUT_MODE = REVIEW_AND_REWRITE

ALLOW_GLOBAL_RESTRUCTURING_PROPOSALS = true
ALLOW_DIRECT_EDITS_OUTSIDE_TARGET_SECTION = false

LOCAL_READER_MAY_USE_LATER_SECTIONS = false

SOURCE_OF_TRUTH_MODE = VERIFY_AND_REPORT_CONFLICTS
SOURCE_OF_TRUTH_PRIORITY = [
    公開インタフェース定義,
    ABI・IDL・Header,
    実装コード,
    Test,
    生成物,
    既存文書,
    コメント
]

DEMONSTRATIVE_POLICY = ELIMINATE_FROM_EDITABLE_PROSE
REQUIRED_EDITABLE_DEMONSTRATIVE_COUNT_AFTER = 0
SEMANTIC_PRESERVATION_POLICY = STRICT

FACT_OPINION_POLICY = STRICT_SENTENCE_LEVEL_SEPARATION
NORMATIVE_LANGUAGE_POLICY = STRICT
STYLE = 常体
PUNCTUATION = 「，．」
```

`DOCUMENT_FAMILY`には，次のいずれかを指定する．

```text
MANUAL
REFERENCE_SET
DEVELOPER_GUIDE
OPERATOR_GUIDE
DESIGN_SPECIFICATION
IMPLEMENTATION_GUIDE
PORTING_GUIDE
```

`TARGET_SECTION_TYPE`には，次のいずれかを指定する．

```text
OVERVIEW
QUICKSTART
TUTORIAL
HOW_TO
PROCEDURE
CONCEPT
EXPLANATION
ARCHITECTURE
DESIGN_SPECIFICATION
REFERENCE
API_REFERENCE
ABI_REFERENCE
OBJECT_REFERENCE
COMMAND_REFERENCE
CONFIGURATION_REFERENCE
SECURITY
TROUBLESHOOTING
COMPATIBILITY
PORTING
GLOSSARY
CHANGELOG
OTHER
```

`TARGET_SECTION_LOCATOR`には，対象セクションを一意に特定するための情報を指定する．
必要に応じて，次の情報を組み合わせてよい．

* セクション名
* ページ名
* ファイル名
* MarkdownまたはLaTeXの見出し
* Anchor
* Label
* 節番号
* 開始箇所と終了箇所
* 複数ファイルにまたがる場合のファイル一覧
* 対象範囲の先頭文および末尾文

`OUTPUT_MODE`は，次のいずれかとする．

```text
REVIEW_ONLY
REVIEW_AND_REWRITE
```

対象セクションを一意に識別できない場合は，推測してレビューを開始してはならない．
候補となる範囲，識別できない理由，追加で必要な情報を示せ．

# 任務

Prismプロジェクトに含まれる技術文書，仕様，ソースコード，Header，IDL，Test，図表を参照し，
`TARGET_SECTION_NAME`で指定されたセクションをレビューおよび改稿せよ．

対象文書は，A9N MicrokernelのManual，Reference，Developer Guide，
Architecture Guide，API Reference，ABI Reference等を想定する．

今回の詳細なレビューおよび書換えの対象は，
`TARGET_SECTION_NAME`で指定されたセクションに限定する．

文書全体を一度に添削してはならない．

一方で，対象セクションを文書全体および実装から切り離して評価してもならない．
プロジェクト全体を確認し，次の事項を把握した上で対象セクションを評価せよ．

* 文書全体の目的
* 想定読者
* 読者が達成すべき作業または理解すべき事項
* 製品またはシステムの対象範囲
* 対応Version
* 対応Architecture
* 前提条件
* 文書全体の情報構造
* 対象セクションが担う役割
* 対象セクションと他セクションとの依存関係
* 対象セクションが説明する実装，API，ABI，Object，CommandまたはProcedure
* 文書上の仕様と実装コードとの対応
* 文書上の仕様とTestとの対応
* 文書上の仕様と公開インタフェースとの対応
* 既知の制約
* 互換性
* Security上の前提
* Failure Mode
* Error Semantics
* Version間差分

プロジェクト全体の情報は，対象セクションの正確性と整合性を確認するために利用する．
後続セクションまたはソースコードの情報を，
対象セクション内の説明不足を見逃すために利用してはならない．

# 論文レビューから技術文書レビューへの変更

技術文書へ，論文固有の評価基準を機械的に適用してはならない．

原則として，次の項目を要求してはならない．

* Research Gap
* 学術的新規性
* 論文としてのContribution
* Related Workによる位置付け
* 査読者を説得するためのMotivation
* 学術的仮説
* 投稿採否
* 国際会議相当の評価実験
* 研究成果としての一般化

設計仕様書またはArchitecture文書では，
設計理由，代替案，Trade-off，制約，Invariantを要求してよい．

性能値を掲載する技術文書では，
測定条件，対象Version，Hardware，統計量，適用範囲を要求せよ．

技術文書の品質は，主として次の基準で評価せよ．

* 正確性
* 完全性
* 一貫性
* 検索性
* 参照性
* 作業可能性
* 誤操作防止
* 契約の明確性
* Version適合性
* 実装との整合性
* 読者適合性
* 保守性
* 更新可能性
* 文書種別への適合性

# 文書種別を混在させてはならない

Manualは，複数種別のページを含む文書集合である．
個々のページまたはセクションには，一つの主要な役割を割り当てよ．

対象セクションについて，次の種別のうち一つを主要種別として特定せよ．

## Tutorial

Tutorialは，学習者が操作を実行しながら概念と技能を獲得するための文書である．

Tutorialでは，次を重視せよ．

* 学習目標
* 成功する一連の体験
* 最初から最後まで完結する手順
* 学習順序
* 最小限の前提知識
* 実行可能な例
* 各段階の確認方法
* 学習を妨げない説明量

Tutorialへ網羅的なReferenceを混入させてはならない．

## How-to Guide

How-to Guideは，既に目的を持つ読者が具体的な作業を達成するための文書である．

How-to Guideでは，次を重視せよ．

* 達成する目的
* 適用条件
* 前提条件
* 必要な入力
* 実行順序
* 分岐条件
* 成功確認
* Failure時の判断
* 副作用
* Rollback
* 関連ReferenceへのLink

How-to Guideへ長い背景説明，歴史，網羅的なOption一覧を混入させてはならない．

## Procedure

Procedureは，正確な順序で実行すべき操作を記述する文書である．

Procedureでは，次を重視せよ．

* 開始状態
* 終了状態
* 実行主体
* 必要権限
* 作業環境
* 順序依存性
* 各StepのAction
* 各StepのExpected Result
* Error時の処理
* 中断可能点
* 再実行可能性
* Idempotency
* Rollback
* Destructive Operationに対するWarning

## Reference

Referenceは，製品，機構，API，ABI，Object，Command，設定項目等を
正確かつ中立的に記述する文書である．

Referenceでは，次を重視せよ．

* 事実
* 仕様
* 契約
* 正確な名称
* 完全な項目一覧
* 安定した順序
* 検索可能な見出し
* 製品構造と対応する文書構造
* 入力
* 出力
* Error
* Side Effect
* 制約
* Version
* 例

Referenceへ，Tutorial，長い議論，意見，宣伝的表現を混入させてはならない．

## ExplanationまたはConcept

ExplanationまたはConcept文書は，
機構の意味，背景，関係，動作原理を理解させるための文書である．

Explanationでは，次を重視せよ．

* Mental Model
* 構成要素間の関係
* 設計上の前提
* なぜ機構が存在するか
* どの問題を解決するか
* 動作原理
* 制約
* Trade-off
* 適用範囲
* 関連ReferenceへのLink

Explanationへ，網羅的なParameter一覧または長い操作手順を混入させてはならない．

## ArchitectureまたはDesign Specification

ArchitectureまたはDesign Specificationは，
システム構造と設計契約を記録する文書である．

次を重視せよ．

* Requirement
* Non-requirement
* Assumption
* Constraint
* Component
* Responsibility
* Interface
* Control Flow
* Data Flow
* State Transition
* Ownership
* Lifetime
* Invariant
* Trust Boundary
* Privilege Boundary
* Failure Model
* Design Rationale
* Alternative
* Trade-off
* Architecture-dependentな部分
* Architecture-independentな部分

Design Specificationへ学術的新規性を要求してはならない．

# レビュー範囲に関する三層モデル

三つの範囲を明確に区別せよ．

## 1. Context Scope

文書全体，仕様，実装，Testを確認し，
対象セクションの正確性と位置付けを把握する範囲である．

Context Scopeには，次の要素を含めてよい．

* 文書全体
* Navigation
* Table of Contents
* Glossary
* Architecture文書
* API Reference
* ABI Reference
* Source Code
* Public Header
* IDL
* Build Configuration
* Test
* Example
* Diagram
* Generated Documentation
* Release Note
* Changelog

Context Scopeに含まれる対象を，
無差別に文単位で添削してはならない．

## 2. Review and Edit Scope

`TARGET_SECTION_NAME`で指定された範囲である．

Review and Edit Scopeでは，次を詳細に検査せよ．

* セクションの目的
* 対象読者
* 読者の作業または疑問
* 小節構成
* 段落構成
* 各文
* 用語
* 定義
* 指示表現
* 事実と意見の分離
* Normative Statement
* Procedure
* API Contract
* ABI Contract
* Example
* Error
* Warning
* Cross-reference
* Code Block
* Command
* Diagram
* Table
* Version情報
* Compatibility情報
* Security情報

`OUTPUT_MODE = REVIEW_AND_REWRITE`の場合に限り，
Review and Edit Scope内の文章を書き換えてよい．

## 3. Global Restructuring Scope

対象セクションを成立させるために必要な，
文書全体の情報構造変更を提案する範囲である．

`ALLOW_GLOBAL_RESTRUCTURING_PROPOSALS = true`の場合，
次の変更を提案してよい．

* PageまたはSectionの移動
* 文書種別の分離
* TutorialとReferenceの分離
* How-to GuideとExplanationの分離
* 重複定義の統合
* Canonical Referenceの指定
* Glossaryへの用語移動
* 前提条件の前倒し
* 詳細仕様のReferenceへの移動
* Architecture説明の別ページ化
* Error一覧の統合
* Version別情報の分離
* Architecture別情報の分離
* Navigationの再設計
* 見出し階層の変更
* 新規ページの追加
* 廃止ページの削除
* Cross-referenceの追加

ただし，

```text
ALLOW_DIRECT_EDITS_OUTSIDE_TARGET_SECTION = false
```

であるため，対象外セクションを無断で書き換えてはならない．

# 最重要原則：全体理解と局所理解を分離せよ

LLMは，後続ページ，実装コード，Testを参照すると，
対象セクション単体では不明瞭な文章であっても意味を推測できる．

推測能力を対象セクションの合格判定に利用してはならない．

## A. Local Reader Review

対象セクションを初めて読む読者として評価せよ．

各文，各段落を評価するときに利用してよい情報は，次だけである．

* 対象箇所より前に書かれている内容
* `TARGET_AUDIENCE_BASELINE`で定義された基礎知識
* 対象セクション内で既に定義された内容
* 対象箇所より前に明示的に参照された図表
* 対象箇所より前に明示的に導入された記号
* 明示的に示されたPrerequisite

後続セクション，後続ページ，ソースコード，Testを利用して，
意味，前提，状態，因果関係，操作対象，指示対象を補完してはならない．

次の判断を禁止する．

* 「ソースコードを読めば分かる」
* 「後続ページで説明されるため問題ない」
* 「実装者なら理解できる」
* 「OS開発者なら推測できる」
* 「一般的なAPIなので説明不要である」
* 「Exampleを読めば仕様を推測できる」
* 「Headerを見れば型が分かる」
* 「Error Codeの意味は名称から分かる」
* 「既存利用者なら知っている」
* 「口頭説明を受ければ理解できる」

後続情報によって意味を推測できた場合は，
次の形式で報告せよ．

```text
対象セクション内では説明されていない．
後続ページまたは実装を参照すると意味を推測できるが，
対象セクション単体では契約を確定できない．
```

## B. Global Consistency Review

文書全体，公開インタフェース，実装，Testを参照し，
次の事項を確認せよ．

* 文書の記述が公開インタフェースと一致するか
* 文書の記述が実装と一致するか
* 文書の記述がTestの期待値と一致するか
* 数値定数が一致するか
* 型が一致するか
* 引数順序が一致するか
* Return Valueが一致するか
* Error Codeが一致するか
* Blocking Semanticsが一致するか
* OwnershipとLifetimeが一致するか
* Concurrency Semanticsが一致するか
* Architecture別の差異が一致するか
* Version情報が一致するか
* Deprecated情報が一致するか
* Exampleが現行仕様で動作するか
* Cross-reference先が存在するか
* 用語が文書全体で一貫するか
* 同じ仕様が複数箇所で矛盾していないか
* 対象セクションが文書全体のNavigation上で適切な位置にあるか

Local Reader ReviewとGlobal Consistency Reviewを混同してはならない．

# Source of Truthに関する規則

`SOURCE_OF_TRUTH_PRIORITY`を参照し，
文書，仕様，実装，Test間の矛盾を検出せよ．

矛盾を発見した場合，優先順位だけを根拠として無断で修正してはならない．

次を報告せよ．

* 矛盾する項目
* 各Sourceの記述
* 各SourceのVersion
* 公開契約として扱われているSource
* 実際の動作を示すSource
* 互換性への影響
* 修正候補
* Maintainerによる判断が必要か

実装の現在動作と，公開仕様として保証すべき動作を区別せよ．

次を同一視してはならない．

* 現在の実装
* 公開契約
* 偶然成立している挙動
* 未定義動作
* 将来も維持する保証
* Testによって固定された挙動
* Architecture固有の挙動
* Debug Build固有の挙動

文書が実装詳細を公開契約として誤って固定していないか検査せよ．

# あなたが担う役割

四役を同時に担え．

## 1. OS・Systems分野の上級技術者

次の分野を理解し，技術的正確性を検査せよ．

* Microkernel
* Capability-based System
* IPC
* Scheduling
* Address Space
* Page Table
* Virtual Memory
* Interrupt
* Notification
* System Call
* ABI
* Concurrency
* Multiprocessor
* Virtualization
* Hypervisor
* Protection
* Isolation
* Security
* Computer Architecture
* Low-level Systems Programming
* Performance Counter
* Boot
* Device Driver
* Resource Management

## 2. Technical Writer

次を検査せよ．

* 文書目的
* 想定読者
* 読者の作業
* 情報の提示順序
* 文書種別
* Information Architecture
* Navigation
* Scanability
* Searchability
* Terminology
* Paragraph Design
* Sentence Design
* Example Design
* Warning Design
* Cross-reference
* Versioning
* Maintenance

## 3. Domain-specificな知識を持たない技術読者

読者は，CSとOSの基礎知識を持つものとする．

読者が持つと仮定してよい知識は，次の範囲である．

* CまたはC++の基礎
* CPUとMemoryの基礎
* ProcessとThread
* Virtual Memory
* System Call
* Interrupt
* Page Tableの概要
* Build Toolの基礎
* Command Lineの基礎

読者が持つと仮定してはならない知識は，次の範囲である．

* A9N固有のObject Model
* A9N固有のCapability
* A9N固有のIPC Semantics
* A9N固有のScheduler
* A9N固有のABI
* A9N固有の命名規則
* 未公開の設計判断
* Maintainer間の暗黙知
* 過去の口頭説明
* Source Codeの配置
* 古いVersionの仕様

## 4. MaintainerおよびRelease Engineer

次を検査せよ．

* 文書が更新可能か
* Version依存情報が分離されているか
* 重複した仕様記述がないか
* Canonical Sourceが明確か
* Deprecated項目が識別できるか
* 互換性破壊を検出できるか
* Exampleを自動Testできるか
* Source Code変更時に更新対象を特定できるか
* Architecture別記述が混在していないか

# 批評姿勢

次の規則を厳守せよ．

1. 著者ではなく文書を批判せよ．

2. 読者が推測できることを，説明済みとして扱ってはならない．

3. 技術的記述に問題がある場合，次を区別せよ．

   * 文書が仕様を定義していない
   * 文書内部で矛盾している
   * 文書と公開インタフェースが矛盾する
   * 文書と実装が矛盾する
   * 文書とTestが矛盾する
   * Versionが不明である
   * Architectureが不明である
   * 公開契約か実装詳細か不明である
   * 判断に必要な情報が欠落している
   * 表現が曖昧で複数の解釈が成立する

4. 「分かりにくい」「説明不足」「詳しく書くべき」とだけ指摘してはならない．
   必ず次を示せ．

   * 問題箇所
   * 欠けている情報
   * 読者が停止する位置
   * 成立し得る誤読
   * 誤実装または誤操作の可能性
   * 必要な修正
   * 修正を置く位置
   * 可能であれば具体的な修正文

5. 文体を整える前に，仕様と技術的意味を検査せよ．

6. 意味が定まらない文を，流麗な文章へ書き換えて問題を隠してはならない．

7. 文書，実装，Testに存在しない仕様，値，Error，保証を捏造してはならない．

8. 文書に複数の解釈が存在する場合，最も好意的な解釈を勝手に採用してはならない．

9. 同じ欠陥が繰り返されている場合，対象セクション内の全出現箇所を列挙せよ．

10. 褒めるためだけのコメントを禁止する．
    長所は，改稿時に維持すべき構造または情報を示す場合に限って記述せよ．

11. 簡潔さを理由として，Prerequisite，Error，Warning，Side Effect，制約を削除してはならない．

12. 完全性を理由として，How-to Guideへ無関係なReference情報を混入させてはならない．

13. Referenceの中立性を理由として，必要なWarningを削除してはならない．

14. 対象外セクションに問題を発見しても，対象セクションとの関係がない問題を詳細にレビューしてはならない．

15. 査読結果自体でも，観察，判定，推奨を分離せよ．

# よく設計された技術文書に指示表現は不要である

よく設計され，論理構造が明確で，対象へ安定した名称が与えられた技術文書には，
指示代名詞，指示連体詞，指示副詞，曖昧な指示接続表現は不要である．

指示表現は，参照対象の探索と解決を読者へ委ねる．

編集可能な文章から指示表現を完全に排除せよ．

```text
REQUIRED_EDITABLE_DEMONSTRATIVE_COUNT_AFTER = 0
```

指示表現の削減では不十分である．
指示表現を一件も残してはならない．

指示表現の排除後も，文章は次の要件を満たさなければならない．

* 文法的に成立する
* 原文の意味を維持する
* 操作主体が明確である
* 操作対象が明確である
* 参照対象が明確である
* 原因と結果が明確である
* 前提条件が明確である
* 実行条件が明確である
* 成功条件が明確である
* Error条件が明確である
* 単数と複数を維持する
* 時間的順序を維持する
* Normative Levelを維持する
* 公開契約と実装詳細の区別を維持する
* 事実，推論，推奨の区別を維持する
* 読者が先行文を探索しなくても理解できる
* 不自然な名詞句反復に陥らない

指示表現を削除した結果，意味が曖昧になる文章は不合格である．

指示表現を具体的な名詞句へ機械的に置換した結果，
極端に冗長または不自然になる場合は，
文または段落を再設計せよ．

指示表現を残す選択肢は存在しない．

## 完全排除する表現

少なくとも，次の表現を完全排除の対象とする．

* これ
* この
* これら
* それ
* その
* それら
* ここ
* そこ
* こう
* そう
* こうした
* そうした
* このような
* そのような
* このため
* そのため
* これにより
* それにより
* ここから
* 以上より
* 以上から
* 前者
* 後者
* 前述の
* 上述の
* 下記の
* 同手法
* 同方式
* 同システム
* 同機構
* 同結果
* 同問題
* 同研究
* 同文書
* 当該
* 該当するもの
* 前記
* 後述の

一覧にない表現であっても，
具体的な対象名を記述せずに先行内容を参照する表現は，
指示表現として扱え．

## 完全排除の対象範囲

次の編集可能領域から指示表現を完全に排除せよ．

* 対象セクションの本文
* 見出し
* 箇条書き
* 脚注
* Figure Caption
* Table Caption
* 数式の説明
* Algorithmの説明
* Code Blockの説明
* Warning
* Note
* Error説明
* Exampleの説明
* 修正版として出力する文章

次の領域は，著者が変更できない場合に限り集計対象から除外してよい．

* 直接引用
* Program Code
* 識別子
* Command名
* LaTeX Command
* Label
* Citation Key
* 外部文書の題目
* 固有名詞

除外領域の周囲に置く説明文では，指示表現を使用してはならない．

## 指示表現を排除する方法

指示表現を削除するだけで修正したことにしてはならない．

次の方法から適切な方法を選択せよ．

1. 具体的なObject名へ置換する．
2. 具体的なAPI名へ置換する．
3. 具体的なOperation名へ置換する．
4. 具体的なState名へ置換する．
5. 具体的なError名へ置換する．
6. 具体的な条件へ置換する．
7. 具体的な原因へ置換する．
8. 具体的な結果へ置換する．
9. 定義済み用語へ置換する．
10. 複数対象を列挙する．
11. Section，Page，Figure，Tableを明示する．
12. 二文を統合する．
13. 一文を分割する．
14. 主語を明示する．
15. 段落のTopicを再設計する．
16. 不要な反復文を削除する．
17. 長い名称へ短い正式名称を定義する．
18. 原因を主語とする文へ変更する．
19. 結果を主語とする文へ変更する．
20. 論理関係を表す具体的な動詞を使用する．

指示表現の排除後も意味を維持できない場合，
修正文を捏造してはならない．

次の形式で報告せよ．

```text
AUTHOR INPUT REQUIRED:
曖昧な指示表現：
成立する指示対象：
必要な確定情報：
影響する仕様：
```

# 事実，契約，要求，推奨，説明を分離せよ

対象セクション内の各文について，
文が担う役割を次から選択せよ．

* `DEFINITION`
  用語，Object，State，Operation，記号の定義

* `EXTERNAL_FACT`
  外部仕様または一般的知識によって確認できる事実

* `IMPLEMENTATION_FACT`
  現在の実装から確認できる事実

* `PUBLIC_CONTRACT`
  利用者へ保証する公開仕様

* `PRECONDITION`
  操作開始前に満たす必要がある条件

* `POSTCONDITION`
  正常終了後に成立する条件

* `INVARIANT`
  常に維持される条件

* `NORMATIVE_REQUIREMENT`
  実装者または利用者が従わなければならない要求

* `RECOMMENDATION`
  推奨するが必須ではない操作または構成

* `PROCEDURAL_STEP`
  読者が実行する操作

* `EXPECTED_RESULT`
  操作後に確認すべき結果

* `ERROR_CONDITION`
  Errorが発生する条件

* `WARNING`
  損失，破損，Security問題，Crash等を防ぐ警告

* `RATIONALE`
  設計または手順の理由

* `EXPLANATION`
  動作原理または概念関係の説明

* `EXAMPLE`
  仕様または操作を具体化する例

* `LIMITATION`
  対応しない範囲または制約

* `VERSION_INFORMATION`
  Version，追加，変更，廃止に関する情報

* `OPINION`
  客観的な契約または事実ではない評価

一つの文には，原則として一つの役割だけを与えよ．

特に，次の混在を禁止する．

* 実装事実と公開契約
* 事実と意見
* 仕様と推奨
* 手順と理由
* 操作とExpected Result
* Resultと評価
* Warningと通常説明
* Error条件と回復手順
* 現行Versionの事実と将来予定
* Architecture-independent仕様とArchitecture固有仕様

異なる役割が混在する文は，役割ごとに分割せよ．

## Normative Language

要求の強さを明確に区別せよ．

* `MUST`または「必須」
  違反すると仕様不適合，誤動作，Security問題等が発生する要求

* `MUST NOT`または「禁止」
  実行してはならない操作

* `SHOULD`または「推奨」
  正当な理由がある場合を除いて従うべき要件

* `SHOULD NOT`または「非推奨」
  正当な理由がある場合を除いて避けるべき操作

* `MAY`または「任意」
  利用者または実装者が選択できる事項

* `CAN`または「可能」
  能力または実装上可能な操作

`必須`，`推奨`，`可能`を混同してはならない．

現在の実装が受け付ける操作を，
公開契約として`MAY`と記述してはならない．

未定義動作を，禁止操作または保証されたErrorへ勝手に変換してはならない．

## 評価的修飾の禁止

事実または仕様を述べる文へ，次のような評価語を混入させてはならない．

* 高性能
* 高速
* 低遅延
* 軽量
* 柔軟
* 安全
* 堅牢
* 単純
* 容易
* 効率的
* 便利
* 優れた
* 重要
* 十分
* 大幅
* 実用的
* 直感的
* 自然
* 明らか
* 当然
* 単に
* わずか
* 無視できる

評価語が必要な場合は，次を示せ．

* 評価基準
* 比較対象
* 測定値
* 適用範囲
* 判断主体

Referenceでは，評価語を原則として削除せよ．

ExplanationまたはDesign Specificationでは，
評価語を設計判断またはRationaleとして別文に分離せよ．

# 技術文書全体に対する必須監査

## 1. 文書目的

対象セクションについて，次を一文で復元せよ．

```text
対象読者が，対象セクションを利用して何を理解，判断，実装または実行できるようになるか
```

一文で復元できない場合，
対象セクションの目的が定まっていないと判断せよ．

## 2. 想定読者

次を明示できるか検査せよ．

* 読者の役割
* 読者の既知知識
* 読者が持たない知識
* 必要な権限
* 必要なTool
* 必要なHardware
* 必要なSoftware
* 必要な前提ページ
* 読者が文書を利用する状況

## 3. Scope

次を明示できるか検査せよ．

* 説明対象
* 説明対象外
* 対応Version
* 対応Architecture
* 対応Configuration
* Experimental機能か
* Stable機能か
* Deprecated機能か
* Internal機能か
* Public機能か

## 4. NavigationとInformation Architecture

次を検査せよ．

* Page Titleが内容を一意に表すか
* 見出しから目的を予測できるか
* 同じ情報が複数箇所で権威的に定義されていないか
* Canonical Pageが存在するか
* Cross-reference先が適切か
* Link先が存在するか
* 循環参照がないか
* 読者が目的の情報を見出しから探索できるか
* Referenceの順序が製品構造と対応するか
* How-toの順序が利用者の作業順序と対応するか
* Glossaryが用語定義の墓場になっていないか
* 重要な前提が別ページへ隠れていないか

## 5. 用語

次を検査せよ．

* 正式名称
* 略称
* 初出時の定義
* 大文字と小文字
* 単数と複数
* 日本語と英語の対応
* Code Identifierとの対応
* 同義語の混在
* 同じ語による異なる概念の表現
* Architecture別の意味差
* Version別の意味差

略語を展開しただけで説明を完了してはならない．

## 6. 段落

各段落は，一つのTopicだけを扱え．

各段落について，次を検査せよ．

* 先頭文がTopicを示すか
* Topicと無関係な文がないか
* 文書種別に適した役割か
* 前段落との関係が明確か
* 次段落が必要となる理由が明確か
* 指示表現に依存していないか
* 一段落に仕様，手順，理由，例が混在していないか

## 7. 文

各文について，次を検査せよ．

* 主語
* 動詞
* 目的語
* 操作主体
* 操作対象
* 条件
* 結果
* Error
* Normative Level
* Version
* Architecture
* 認識論的役割
* 指示表現
* 評価語
* 曖昧な副詞
* 過剰な受動態
* 長すぎる修飾
* 二重否定
* 不明確な列挙範囲
* 一文一事項

# ProcedureおよびHow-toの監査

ProcedureまたはHow-to Guideでは，次を検査せよ．

* 目的がTitleから分かるか
* 開始状態が明示されているか
* 終了状態が明示されているか
* Prerequisiteがあるか
* 必要権限があるか
* 必要Toolがあるか
* Working Directoryがあるか
* Shellまたは実行環境があるか
* Architectureがあるか
* Versionがあるか
* CommandがCopy可能か
* Prompt文字とCommandが区別されているか
* Placeholderが説明されているか
* 各StepがActionから始まるか
* Step順序に理由があるか
* 各StepのExpected Resultがあるか
* Expected Resultが観測可能か
* Failure時の分岐があるか
* Destructive OperationのWarningが先にあるか
* Side Effectがあるか
* Idempotencyがあるか
* 再実行条件があるか
* Rollbackがあるか
* 成功確認があるか
* 完了後の状態があるか

手順中に背景説明を長く挿入してはならない．
必要な背景説明はExplanationへ移動し，Linkを設けよ．

# Referenceの監査

Reference Entryは，対象に応じて次の項目を持つべきである．

## 共通Reference Template

```text
名称
分類
Status
導入Version
廃止Version
対応Architecture
概要
役割
公開契約
前提条件
入力
出力
状態変化
副作用
制約
Error
Concurrency
Security
未定義またはReservedな挙動
Example
関連項目
Source of Truth
```

## API Reference Template

```text
API名
Signature
HeaderまたはModule
呼出し主体
Purpose
Precondition
Parameter
Parameter Direction
Valid Range
Nullability
Alignment
Ownership
Lifetime
Return Value
Error
Blocking Semantics
Scheduling Effect
Atomicity
Thread Safety
Reentrancy
Side Effect
Privilege Requirement
Architecture Difference
Version
Example
Related API
```

## ABI Reference Template

```text
ABI名
Version
Architecture
Calling Convention
Register Assignment
Input Register
Output Register
Clobbered Register
Preserved Register
Stack Layout
Alignment
Data Width
Endianness
Padding
Reserved Field
Bit Number
Bit Meaning
Valid Value
Invalid Value
Error Representation
State Transition
Privilege Level
Compatibility Rule
Example
```

## Object Reference Template

```text
Object名
Object Type
Purpose
Authority
Creation
Initial State
Operation
State Transition
Ownership
Lifetime
Transfer
Copy
Revocation
Destruction
Invalidation
Concurrency
Blocking
Error
Security Boundary
Related Object
```

## Command Reference Template

```text
Command名
Synopsis
Purpose
Argument
Option
Default
Environment Variable
Input
Output
Exit Status
Side Effect
Privilege
File
Signal
Error
Example
Version
Related Command
```

# A9N Microkernel固有の監査

該当するセクションでは，次の観点を追加せよ．

## Capability

* Capabilityが表すObject
* Capabilityが付与するAuthority
* Capabilityが付与しないAuthority
* 作成方法
* Copy
* Transfer
* Mint
* Revoke
* Delete
* Lifetime
* Object破棄との関係
* Invalid Capabilityの扱い
* Error
* Concurrency
* Architecture-independentな契約

## Kernel CallおよびIPC

* Operation名
* Kernel Call Number
* 呼出し主体
* 対象Capability
* Register ABI
* Message Register
* IPC Buffer
* Input
* Output
* Tag
* Message Length
* Blocking条件
* Wakeup条件
* Schedulerへの影響
* Address Space切替え
* Reply Authority
* Reply State
* Timeout
* Cancellation
* Error
* State Transition
* Concurrency
* Same-address-spaceとCross-address-spaceの差

## ProcessおよびScheduler

* ProcessまたはThreadの状態
* Ready条件
* Running条件
* Blocked条件
* Priority
* Scheduling Policy
* Preemption
* Yield
* Direct Switch
* Queueへの所属条件
* Wakeup
* Starvation
* SMP Semantics
* CPU Affinity
* Error

## Address Space，Page Table，Frame

* Object間関係
* Page Size
* Alignment
* Virtual Address Range
* Physical Address Range
* Permission
* Execute Permission
* Cache Attribute
* Global Bit
* Mapping
* Unmapping
* Remapping
* Ownership
* TLBへの影響
* Architecture固有属性
* Error
* Invalidation Semantics

## InterruptおよびNotification

* Interrupt Source
* IRQ Number
* Binding
* Delivery Target
* Pending State
* Mask
* Unmask
* Acknowledge
* Re-delivery
* Notification Bit
* Coalescing
* Ordering
* Lossの有無
* Concurrency
* Error

## Virtualization

* VCPU
* VAddressSpace
* VPageTable
* Run Operation
* Entry State
* Exit State
* Exit Reason
* State Descriptor
* Register State
* Control State
* Reserved Bit
* Architecture-dependentな部分
* Architecture-independentなCapability契約
* Error
* Concurrency
* Security Boundary

# ExampleおよびCode Blockの監査

全てのExampleについて，次を検査せよ．

* 現行Versionで動作するか
* 必要なImportまたはIncludeがあるか
* 必要なBuild Optionがあるか
* Placeholderが説明されているか
* Error処理が必要か
* Undefined Behaviorを含まないか
* Privilegeが明示されているか
* Architecture依存か
* 実行結果が示されているか
* 説明対象だけへ集中しているか
* Copy後に不要な修正を要求しないか
* 自動Testへ組み込めるか

Exampleを仕様の唯一の説明にしてはならない．
Exampleから仕様を推測させてはならない．

# Warning，Caution，Note

次を区別せよ．

* `DANGER`
  生命，重大なHardware損傷等の危険

* `WARNING`
  Data Loss，Security侵害，Crash，永続的破損等の危険

* `CAUTION`
  失敗，性能劣化，一時的な不整合等の危険

* `NOTE`
  理解または利用を補助する情報

警告は，危険な操作より前に配置せよ．

警告には，次を含めよ．

* 危険の内容
* 危険が発生する条件
* 危険を回避する操作
* 発生後の回復方法

Warningを通常段落へ埋没させてはならない．

# VersionおよびCompatibility

次を検査せよ．

* 文書が対象とするVersion
* 機能の導入Version
* 変更Version
* 廃止Version
* Compatibility Guarantee
* Architecture差分
* Configuration差分
* Experimental Status
* Deprecated Status
* Migration Path
* 古いExample
* 古い名称
* 古いError Code
* 古いABI Layout

「現在」「最新」「将来」等の相対表現を避け，
具体的なVersionまたはReleaseを記述せよ．

# 実行手順

レビューは，必ず五段階で実施せよ．

## Pass 1：Document Reconnaissance

文書全体を読み，次を復元せよ．

* 文書集合の目的
* 想定読者
* 文書種別
* 読者の主要Task
* Product Model
* 文書構造
* Canonical Reference
* Glossary
* 対応Version
* 対応Architecture
* Source of Truth
* 対象セクションの役割

対象外セクションの詳細な文章添削を始めてはならない．

## Pass 2：Local Constrained Reading

対象セクションを先頭から一文ずつ読み直せ．

後続ページ，実装，Testから得た情報を一旦利用禁止とする．

次の最初の発生位置を特定せよ．

* 読者が目的を理解できない
* 読者が対象を特定できない
* 読者が操作主体を特定できない
* 読者がPrerequisiteを特定できない
* 読者が指示対象を特定できない
* 読者がNormative Levelを特定できない
* 読者が成功条件を特定できない
* 読者がError時の挙動を特定できない
* 未定義語が現れる
* 事実と意見が混在する
* 公開契約と実装詳細が混在する
* Versionが不明になる
* Architectureが不明になる

## Pass 3：Technical Verification

文書，公開インタフェース，実装，Testを照合せよ．

次を確認せよ．

* 名称
* 型
* 数値
* Constant
* Flag
* Bit
* Offset
* Register
* Operation Number
* Error Code
* State
* Transition
* Return Value
* Blocking
* Ownership
* Lifetime
* Version
* Architecture

## Pass 4：Information Architecture Review

対象セクションの内容が，
適切な文書種別と位置に配置されているか確認せよ．

Tutorial，How-to，Reference，Explanationの混在を特定せよ．

## Pass 5：Rewrite and Validation

対象セクションを改稿し，
技術的正確性，文書種別，指示表現完全排除，
事実と意見の分離を再検査せよ．

# 出力形式

指定した順序で出力せよ．

## 0. 対象範囲

* 対象文書
* 対象セクション
* 対象セクション種別
* 対象ファイル
* 開始位置
* 終了位置
* 参照したSource
* 確認できなかったSource
* 対応Version
* 対応Architecture
* Buildまたは生成状態

## 1. 文書分類

次を示せ．

* Document Family
* 対象セクションの主要種別
* 混在している副次種別
* 想定読者
* Reader Goal
* Prerequisite
* 対象範囲
* 対象外
* 公開契約か内部文書か

## 2. 総合判定

判定は，次から選択せよ．

* 公開禁止：誤実装，Security問題，Data Loss等を招く重大な誤りがある
* 使用不能：読者が目的を達成できない
* 根本的な再構成が必要
* 大幅改稿が必要
* 局所的な改稿が必要
* 公開可能

次を示せ．

* 判定
* 判定理由
* 最も重大な問題3件
* 読者が最初に失敗する箇所
* Maintainerが最初に修正すべき箇所
* 維持すべき要素を最大3件

## 3. Reader Task Model

次を復元せよ．

* 読者の役割
* 読者が持つ知識
* 読者が持たない知識
* 読者の目的
* 開始状態
* 終了状態
* 必要な判断
* 必要な操作
* 必要なReference
* 失敗し得る地点

復元できない項目は，
「対象セクションから一意に復元できない」と記載せよ．

## 4. Local Reader Review

後続ページ，実装，Testを利用せずに評価せよ．

### 最初の理解破綻点

* 位置
* 原文
* 読者が既に知っている情報
* 不足情報
* 成立する解釈
* 誤操作または誤実装
* 必要な修正

### 理解破綻の伝播

最初の欠陥が後続の説明，操作，判断へ与える影響を示せ．

### 後続情報による遡及的補完

| 対象箇所 | 対象セクション内で不足する情報 | 補完可能なSource | 推測可能か | 局所的には合格か |
| ---- | --------------- | ----------- | ----- | -------- |

## 5. Source of TruthおよびDrift監査

| ID | 項目 | 文書 | 公開IF | 実装 | Test | Version | 矛盾 | 推奨処理 |
| -- | -- | -- | ---- | -- | ---- | ------- | -- | ---- |

次を区別せよ．

* 文書が誤っている
* 実装が仕様へ適合していない
* Testが古い
* Version差分
* Architecture差分
* 公開契約が不明
* Maintainer判断が必要

## 6. Contract Matrix

対象セクション内の仕様記述を抽出せよ．

| ID | 記述 | 種類 | 対象 | 条件 | 保証 | Source | Version | 完全性 | 問題 |
| -- | -- | -- | -- | -- | -- | ------ | ------- | --- | -- |

`種類`には，次を使用せよ．

* Definition
* Public Contract
* Precondition
* Postcondition
* Invariant
* Requirement
* Recommendation
* Procedure
* Error
* Warning
* Limitation
* Version Information
* Implementation Detail

## 7. 段落単位の監査

| 段落 | 本来の役割 | 現在の役割 | 文書種別 | Topic Sentence | 新規情報 | 問題 | 判定 |
| -- | ----- | ----- | ---- | -------------- | ---- | -- | -- |

各段落について，次を示せ．

* 前段落との接続
* 次段落との接続
* 不足するPrerequisite
* 不足する仕様
* 不足するError
* 冗長な説明
* 別ページへ移すべき内容
* 指示表現
* 役割の混在
* 推奨する役割

## 8. 用語監査

| 用語 | 初出位置 | 定義 | Code上の名称 | 表記揺れ | Version差 | 問題 | 推奨 |
| -- | ---- | -- | -------- | ---- | -------- | -- | -- |

加えて，次を示せ．

* 未定義語
* 略語
* 同義語
* 衝突する語
* Architecture別用語
* Deprecated用語
* Glossaryへ移すべき語

## 9. 指示表現完全排除監査

編集可能な文章に含まれる指示表現を全て列挙せよ．

| ID | 位置 | 原文 | 指示表現 | 指示対象候補 | 排除方法 | 修正後 | 意味保存 |
| -- | -- | -- | ---- | ------ | ---- | --- | ---- |

指示表現を残す判定を出してはならない．

集計結果を次の形式で示せ．

```text
EDITABLE_DEMONSTRATIVE_COUNT_BEFORE:
<number>

EDITABLE_DEMONSTRATIVE_COUNT_AFTER:
0

EXCLUDED_NON_EDITABLE_OCCURRENCES:
<location and reason>

SEMANTIC_PRESERVATION_FAILURE_COUNT:
0
```

## 10. 文の役割監査

| 文ID | 位置 | 原文 | 役割 | 根拠 | 混在 | 評価語 | Normative Level | 問題 | 修正 |
| --- | -- | -- | -- | -- | -- | --- | --------------- | -- | -- |

次を個別に列挙せよ．

* 事実と意見の混在
* 公開契約と実装詳細の混在
* RequirementとRecommendationの混在
* StepとExpected Resultの混在
* ErrorとRecoveryの混在
* 現行仕様と将来予定の混在
* Architecture差分の混在
* 根拠のない評価語
* 根拠のないSecurity Claim
* 根拠のない性能Claim

## 11. 詳細指摘

各指摘を次の形式で記述せよ．

### [指摘ID] [重要度] [分類] [確信度]

* 位置：
* 原文：
* 観察：
* 技術的根拠：
* Source of Truth：
* 読者への影響：
* 誤操作または誤実装の可能性：
* 判定：
* 必須修正：
* 推奨修正：
* 修正文：
* 意味保存確認：
* Maintainer判断の必要性：

重要度は，次から選択する．

* `P0/CRITICAL`
  誤実装，ABI破壊，Security問題，Data Loss，Crash等を招く問題

* `P1/MAJOR`
  読者が作業を完了できない，または仕様を一意に解釈できない問題

* `P2/MINOR`
  局所的な不正確さ，検索性，理解性，保守性の問題

* `P3/STYLE`
  文体，表記，見出し，版面の問題

分類は，次から選択する．

* Document Purpose
* Audience
* Scope
* Document Type
* Information Architecture
* Navigation
* Technical Correctness
* Public Contract
* Implementation Drift
* API
* ABI
* Object Model
* Procedure
* Prerequisite
* Error
* Warning
* Security
* Concurrency
* Ownership
* Lifetime
* Version
* Compatibility
* Architecture
* Example
* Terminology
* Paragraph
* Sentence
* Demonstrative Expression
* Fact–Opinion Separation
* Normative Language
* Cross-reference
* Formatting

## 12. 全体構成変更提案

| ID | 必要度 | 操作 | 移動元 | 移動先 | 内容 | 理由 | 関連指摘 |
| -- | --- | -- | --- | --- | -- | -- | ---- |

次を区別せよ．

* 対象セクション内だけで解消可能
* Tutorialへ移すべき内容
* How-toへ移すべき内容
* Referenceへ移すべき内容
* Explanationへ移すべき内容
* Glossaryへ移すべき内容
* Architecture別に分離すべき内容
* Version別に分離すべき内容
* Maintainer判断が必要な内容

## 13. 推奨セクション構成

段落またはReference Entry単位で再設計せよ．

各単位について，次を示せ．

```text
Unit N
役割：
文書種別：
読者の目的：
中心情報：
前提：
契約：
必要な例：
必要なWarning：
関連Reference：
排除すべき指示表現：
```

## 14. 修正案

`OUTPUT_MODE = REVIEW_AND_REWRITE`の場合に出力せよ．

### 14.1 最小修正案

現在の構造を可能な限り維持し，
P0およびP1を解消する修正案を提示せよ．

編集可能な文章から指示表現を完全に排除せよ．

### 14.2 推奨修正案

文書種別，読者の目的，仕様構造から再設計した修正版を提示せよ．

修正版は，次の規則に従う．

* 対象セクションだけを書き換える
* 常体を使用する
* 句読点は「，．」を使用する
* 指示表現を一件も使用しない
* 指示表現の排除後も意味を維持する
* 公開契約を捏造しない
* Errorを捏造しない
* Versionを捏造しない
* 保証範囲を拡大しない
* 実装詳細を公開契約へ変換しない
* RequirementとRecommendationを区別する
* Architecture差分を明示する
* Version差分を明示する
* 操作主体を明示する
* 操作対象を明示する
* 前提条件を明示する
* 成功条件を明示する
* Error条件を明示する
* 副作用を明示する
* Ownershipを明示する
* Lifetimeを明示する
* Blocking Semanticsを明示する
* Code Identifierを維持する
* API名を維持する
* ABI数値を変更しない
* Bit Numberを変更しない
* Offsetを変更しない
* Error Codeを変更しない
* Commandを勝手に変更しない
* Sourceに存在しない情報を追加しない
* 事実と意見を分離する
* 公開契約と実装詳細を分離する
* StepとExpected Resultを分離する
* Warningを危険操作より前に置く
* 一文一事項とする
* 一段落一Topicとする

情報が不足する場合は，次の形式で示せ．

```text
AUTHOR INPUT REQUIRED:
不足情報：
必要な理由：
影響する契約：
挿入位置：
保守的な代替記述：
```

### 14.3 変更対応表

| 修正箇所 | 元の問題 | 指摘ID | 修正内容 | Source | 意味保存 | 契約変更 |
| ---- | ---- | ---- | ---- | ------ | ---- | ---- |

## 15. 修正後の検証

修正版について，次を再検査せよ．

* 読者が目的を理解できるか
* 対象範囲が明確か
* 対応Versionが明確か
* 対応Architectureが明確か
* Prerequisiteが明確か
* 公開契約が明確か
* 実装詳細と区別されているか
* Operationの入力と出力が明確か
* Errorが明確か
* OwnershipとLifetimeが明確か
* BlockingとConcurrencyが明確か
* Exampleが仕様と一致するか
* Cross-referenceが有効か
* 指示表現が0件か
* 指示表現排除後も意味を維持するか
* 事実と意見が分離されているか
* RequirementとRecommendationが分離されているか
* 根拠のない評価語がないか
* 文書種別が混在していないか
* 文書全体との矛盾がないか
* 実装との矛盾がないか
* Testとの矛盾がないか

## 16. Scorecard

各項目を0から5で評価し，根拠を一文で示せ．

* 文書目的
* 想定読者
* Reader Goal
* Scope
* 文書種別への適合性
* 技術的正確性
* 公開契約の明確性
* 実装との整合性
* 完全性
* Prerequisite
* Error Semantics
* Warning
* OwnershipとLifetime
* Concurrency
* Version
* Compatibility
* Architecture差分
* Navigation
* Searchability
* 用語の一貫性
* Paragraph Design
* Sentence Clarity
* 指示表現の完全排除
* 指示表現排除後の意味保存
* 事実と意見の分離
* Normative Language
* Example
* 保守性

評価基準は，次のとおりとする．

* 0：存在しない，または評価不能
* 1：根本的に破綻している
* 2：重大な欠陥がある
* 3：最低限成立しているが改稿が必要
* 4：十分に利用可能
* 5：模範的

単純平均だけで総合判定を決めてはならない．

技術的正確性，公開契約，Error Semantics，
指示表現完全排除，意味保存のいずれかが破綻している場合，
総合判定へ直接反映せよ．

## 17. Maintainerへの確認事項

文書の改善に不可欠だが，Sourceから判断できない事項だけを列挙せよ．

各質問について，次を示せ．

* 質問
* 必要な理由
* 影響する仕様
* 影響するVersion
* 回答がない場合の保守的な記述
* 関連指摘ID

## 18. Documentation Audit Result

最後に，必ず次の形式で出力せよ．

```text
DOCUMENTATION_AUDIT_RESULT

Target section:
<name>

Document type:
<type>

Technical conflicts:
<number>

Unresolved public-contract ambiguities:
<number>

Missing prerequisites:
<number>

Missing error semantics:
<number>

Editable demonstratives before revision:
<number>

Editable demonstratives after revision:
0

Semantic preservation failures:
0

Sentences mixing facts and opinions after revision:
0

Sentences mixing public contracts and implementation details after revision:
0

Unclassified sentences after revision:
0

Unsupported evaluative modifiers after revision:
0

Broken cross-references:
<number>

Verdict:
PASS or FAIL
```

次の全条件を満たす場合に限り，`PASS`とせよ．

```text
重大な技術的矛盾が存在しない
公開契約を一意に解釈できる
必要なPrerequisiteが存在する
必要なError Semanticsが存在する
編集可能な文章に指示表現が存在しない
指示表現の排除後も意味が維持されている
事実と意見が分離されている
公開契約と実装詳細が分離されている
全ての文の役割を分類できる
根拠のない評価語が存在しない
重大なCross-reference切れが存在しない
```

# 完了条件

次の条件を満たすまで，レビューを完了したと宣言してはならない．

* 対象セクションを一意に特定した
* 文書種別を特定した
* 想定読者を特定した
* Reader Goalを復元した
* Context Scopeを確認した
* Local Reader Reviewを実施した
* Global Consistency Reviewを実施した
* Source of Truthを照合した
* 文書と実装の矛盾を確認した
* 文書とTestの矛盾を確認した
* 全てのP0およびP1を列挙した
* 対象セクション内の全段落を確認した
* 対象セクション内の全ての仕様記述を分類した
* 対象セクション内の全指示表現を抽出した
* 編集可能な文章から指示表現を完全に排除した
* 指示表現排除後も意味を維持した
* 事実と意見を分離した
* 公開契約と実装詳細を分離した
* RequirementとRecommendationを分離した
* Prerequisiteを確認した
* Error Semanticsを確認した
* OwnershipとLifetimeを確認した
* Versionを確認した
* Architecture差分を確認した
* Exampleを確認した
* Cross-referenceを確認した
* 文書種別の混在を確認した
* 必要な全体構成変更を独立して提示した
* 対象外セクションを無断で書き換えていない
* Sourceにない仕様，値，保証を生成していない
* 修正版を再検査した
* Documentation Audit Resultを出力した

# 出力長の制約がある場合

一度に完了できない場合は，次の順に優先して出力せよ．

1. 対象範囲
2. 文書分類
3. 総合判定
4. P0およびP1
5. Source of TruthおよびDrift監査
6. Contract Matrix
7. Local Reader Review
8. 指示表現完全排除監査
9. 文の役割監査
10. 全体構成変更提案
11. 推奨セクション構成
12. 修正版
13. P2およびP3
14. Scorecard
15. Documentation Audit Result

中断する場合は，最後に必ず次の形式で記載せよ．

```text
REVIEWED THROUGH:
<最後に確認したファイル，ページ，節，段落>

CONTINUE FROM:
<次に確認するファイル，ページ，節，段落>

UNFINISHED ITEMS:
<未完了の出力項目>
```

未確認範囲を，確認済みであるかのように扱ってはならない．

