#import "/components/layout.typ" : template
#import "/components/api_table.typ" : api_table

#show: doc => template(
    [generic],
    doc,
)

= Generic

== Introduction

Genericは, メモリを抽象化するCapabilityです.

A9Nカーネルはヒープを持たないため, カーネルオブジェクトのようなシステム内で使用するメタデータのメモリは, ユーザーが明示的に割り当てる必要があります.

生の物理メモリをユーザーに直接使用させるのはセキュリティ上のリスクが発生するため, `convert()`メカニズムを用いて安全な割当ポリシーを実現します.
`convert()`は対象Genericを切り出し, カーネルオブジェクトを作成します. 作成したオブジェクトは親GenericのDependency Nodeに登録され, 初期化処理などに使用されます.


== Generic API

=== `convert`
```cpp
common::error convert(
    a9n::capability_descriptor  generic_descriptor, 
    a9n::kernel::capability_type               type,               
    a9n::word                                  size,               
    a9n::word                                 count,              
    a9n::capability_descriptor      node_descriptor,    
    a9n::word                            node_index,         
)
```

#api_table(
    "generic_descriptor", "対象GenericへのDescriptor",
    "type", "作成するCapabilityのType",
    "size", "作成するCapabilityのSize",
    "count", "作成するCapabilityの個数",
    "node_descriptor", "格納先NodeへのDescriptor",
    "node_index", "格納先NodeのIndex",
)
