# 未踏IT人材発掘・育成事業 成果報告書 Template

このTypst Templateは未踏IT人材発掘・育成事業における成果報告書を作成するためのものです．

## How to Write

### 1. `main.typ`にProjectの情報を記述

```typ
#template(
    project_name: [天才的なアイデアによる未踏的なシステムの実装],
    creator: "未踏 太郎",
    project_manager: "未踏 花子",
    year: 2024,
    delivery_date: datetime(year: 2025, month: 3, day: 7),
    contract_number: "000",
    [
        #include "/pages/summary.typ"
        #include "/pages/background_and_objectives.typ"
        #include "/pages/project_overview.typ"
        #include "/pages/development_details.typ"
        #include "/pages/features_of_development_results.typ"
        #include "/pages/future_challenges_and_prospects.typ"
        #include "/pages/differences_from_the_original_implementation_plan.typ"
        // 複数人プロジェクトの場合に使用
        // #include "/pages/development_responsibilities.typ"
        #include "/pages/self_analysis_of_glowth.typ"
        // 秘匿ノウハウが含まれる場合に使用
        // #include "/pages/designation_of_confidential_know_how.typ"
        // 特記事項が存在する場合に使用
        // #include "/pages/others.typ"
        #include "/pages/appendix.typ"
    ],
)
```

- `project_name`にプロジェクト名を記述します
- `creator`にクリエータ名を記述します
- `project_manager`にプロジェクトマネージャ名を記述します
- `year`に採択年度を記述します
- `delivery_date`に納品日を記述します
- `contract_number`に契約番号を記述します

> [!NOTE]
> コメントアウトされている部分は必要に応じて有効化してください.

### 2. 各ページの内容を記述

- `pages/summary.typ` : 要約
- `pages/background_and_objectives.typ` : 背景及び目的
- `pages/project_overview.typ` : プロジェクト概要
- `pages/development_details.typ` : 開発内容
- `pages/features_of_development_results.typ` : 開発成果の特長
- `pages/future_challenges_and_prospects.typ` : 今後の課題，展望
- `pages/differences_from_the_original_implementation_plan.typ` : 実施計画書内容との相違点
- `pages/development_responsibilities.typ` : 開発分担 (Additional)
- `pages/self_analysis_of_glowth.typ` : 成長の自己分析
- `pages/designation_of_confidential_know_how.typ` : 秘匿ノウハウの指定 (Additional)
- `pages/others.typ` : 特記事項 (Additional)
- `pages/appendix.typ` : 付録

### 3. Build

下記の手順に従ってください.

## Build

### Dependency

- Typst
- Make

### Compile

通常のCompileを実行します．`build/private_detailed_report.pdf`が生成されます．

```bash
make
```
### Hot Reload

Typstの機能によりHot Reload（ファイル変更時のAuto Re-Compile）を実行します．

```bash
make watch
```

## Additional Settings

- Fontは`components/layout.typ`の`font settings` Sectionで設定可能です.
使用可能なFontは`typst fonts`コマンドで確認できます.

> [!IMPORTANT]
> デフォルトから変更することを強く推奨します!
> Bodyには視認性の高い日本語フォントを，Codeには等幅フォントを使用することを推奨します.

### Example

```typ
// font settings
// body
set text(
    font: "FOT-TsukuAOldMin Pr6N",
)
// code 
show raw: set text(font: "Hack")
```
