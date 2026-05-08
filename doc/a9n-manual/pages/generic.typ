#import "/components/api_table.typ" : *

= Generic

== Introduction

Genericはメモリを抽象化するCapabilityです.

A9N Microkernelはヒープを持ちません. 従って, 必要なKernel ObjectをUserが明示的に割り当てる必要があります.

User-LevelのKernel Object割当ては, Genericによる`convert()`メカニズムを用いて安全に実現されます.
`convert()`によって作成したオブジェクトは親となるGenericのDependency Nodeに登録され, 初期化処理などに使用されます.


== Generic API

=== `convert`

#api_table(
    "capability_descriptor", "generic_descriptor", "対象GenericへのDescriptor",
    "capability_type", "type", "作成するCapabilityのType",
    "word", "specific_bits", [Capability作成時に使用する固有Bits \ cf., @specific_bits],
    "word", "count", "作成するCapabilityの個数",
    "capability_descriptor", "node_descriptor", "格納先NodeへのDescriptor",
    "word", "node_index", "格納先NodeのIndex",
)

==== `specific_bits` <specific_bits>

#normal_table(
    "Capability Node", [NodeのSlot数を表すRadix ($"count" = 2^"specific_bits"$)],
    "Generic", [GenericのSizeを表すRadix ($"size" = 2^"specific_bits"$)],
    "Process Control Block", "-",
    "IPC Port", "-",
    "Interrupt Port", "-",
    "Page Table", "depth",
    "Frame", "-",
    "Virtual CPU", "-",
    "Virtual Page Table", "-",
)
