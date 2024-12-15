#import "/components/layout.typ" : template

#show: doc => template(
    [capability node],
    doc,
)

= Capability Node

== Introduction
*Capability Node*は, Capabilityを格納するための*コンテナ*として使用されるCapabilityです. \
このNodeは$2^"radix"$個のSlotを持つRadix Treeです. \
子としてNodeが保持可能であり, 複数階層のCapability Treeを作成できます.

== Addressing
Node内のCapabilityはDescriptorによって, 以下のようにAddressingされます :
+ Descriptorの先頭8bitを取り出し, `depth_bits`とします
  - `depth_bits`は探索可能bit数の最大値を表します
  - 例えば, 64bit Computerでは$64 - 8$の56bitが標準の`depth_bits`となります
+ Node内の`radix_bits`からIndexに使用するBitを決定します
+ Descriptorから2で得たIndex分のbitを取り出し, 子を取得します
+ 子に対して, Descriptorを使い切るか終端に到達するまで再帰的に探索を行います

NodeとDescriptorはPage TableとVirtual Addressのような構造をしています.

== Node API

=== `copy`
=== `move`
=== `remove`
=== `revoke`
