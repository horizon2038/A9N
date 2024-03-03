#import "layout.typ" : template

#show: doc => template(
    [capability node],
    doc,
)

= Capability Node

== Introduction
Capability Nodeは, Capabilityを格納するための*コンテナ*として機能するCapabilityです.

Nodeのスロット数は作成時に規定されますが, 複数階層のNodeを使用することにより, ユーザー側で拡張することが可能です.

Node内のCapabilityはDescriptorによって, 以下のようにAddressingされます :
- Node内のRadix Bitsから, Indexに使用するBitを決定します
- DescriptorからIndexを取り出し, 子ノードを取得します
- 子ノードに対して, Descriptorを使い切るか, 終端に到達するまで再帰的に探索を行います

NodeとDescriptorはPage TableとVirtual Addressのような構造をしています.

Node自体を指定したい場合など, Descriptorの残りに関わらず探索を途中で打ち切りたい場合, Node Depthを設定することで探索の上限を設けることが可能です. 

== Node API

=== `copy`
=== `move`
=== `remove`
=== `revoke`
