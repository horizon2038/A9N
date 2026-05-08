#import "/components/api_table.typ" : api_table

= Capability Node

== Introduction

*Capability Node*は, Capabilityを格納するための*コンテナ*として使用されるCapabilityです. \
このNodeは$2^"radix"$個のSlotを持つRadix Treeです. \
子としてNodeが保持可能であり, 複数階層のCapability Treeを作成できます.

== Addressing

Node内のCapabilityは*Capability Descriptor*を用いてAddressingされます :
+ Descriptorの先頭8bitを取り出し, `depth_bits`とします
  - `depth_bits`は探索可能bit数の最大値を表します
  - 例えば, 64bit Computerでは$64 - 8$の56bitが標準の`depth_bits`となります
+ Node内の`radix_bits`からIndexに使用するBitを決定します
+ Descriptorから2で得たIndex分のbitを取り出し, 子を取得します
+ 子に対して, Descriptorを使い切るか終端に到達するまで再帰的に探索を行います

\

NodeとDescriptorはPage TableとVirtual Addressのような構造をしています.

== Node API

=== `copy`

CapabilityのCopyを行います. *Rights*はそのままCopyされます.

#api_table(
    "capability_descriptor", "node_descriptor", "対象Capability NodeへのDescriptor",
    "word", "destination_index", "DestinationとなるCapabilityを格納しているNode内Index",
    "capability_descriptor", "source_descriptor", "SourceとなるNodeのDescriptor",
    "word", "source_index", "SourceとなるNodeのCapabilityを格納しているIndex",
)

=== `move`

CapabilityのMoveを行います. *Rights*はそのままMoveされます.

#api_table(
    "capability_descriptor", "node_descriptor", "対象Capability NodeへのDescriptor",
    "word", "destination_index", "DestinationとなるCapabilityを格納しているNode内Index",
    "capability_descriptor", "source_descriptor", "SourceとなるNodeのDescriptor",
    "word", "source_index", "SourceとなるNodeのCapabilityを格納しているIndex",
)

=== `mint`

CapabilityのMintを行います.
新しい*Rights*は元となるRightsのSubsetである必要があります. 従って, より高いRightsを持つCapabilityを作成することはできません.

#api_table(
    "capability_descriptor", "node_descriptor", "対象Capability NodeへのDescriptor",
    "word", "destination_index", "DestinationとなるCapabilityを格納しているNode内Index",
    "capability_descriptor", "source_descriptor", "SourceとなるNodeのDescriptor",
    "word", "source_index", "SourceとなるNodeのCapabilityを格納しているIndex",
    "capability_rights", "new_rights", "新しいRights",
)

=== `demote`

CapabilityのRightsを降格します.
この操作は不可逆であり, 一度降格したRightsを昇格することはできません.

#api_table(
    "capability_descriptor", "node_descriptor", "対象Capability NodeへのDescriptor",
    "word", "target_index", "対象のCapabilityを格納しているNode内Index",
    "capability_rights", "new_rights", "新しいRights",
)

=== `remove`

CapabilityのRemoveを行います. 
Dependency NodeにSiblingが存在しない場合, Revokeも実行し完全に削除されます.

#api_table(
    "capability_descriptor", "node_descriptor", "対象Capability NodeへのDescriptor",
    "word", "target_index", "削除対象のCapabilityを格納しているNode内Index"
)

=== `revoke`

CapabilityのRevokeを行います. 

#api_table(
    "capability_descriptor", "node_descriptor", "対象Capability NodeへのDescriptor",
    "word", "target_index", "削除対象のCapabilityを格納しているNode内Index"
)
