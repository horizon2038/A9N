#import "layout.typ" : template

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
    library::capability::capability_descriptor generic_descriptor, 
    library::capability::capability_type       type,               
    library::common::word                      size,               
    library::common::word                      count,              
    library::capability::capability_descriptor node_descriptor,    
    library::common::word                      node_depth,         
    library::common::word                      node_index,         
)
```

#table(
    columns: 2,
    inset: (
        x: 1.5em,
        y: 1em,
    ),
    align: (x, y) => ((left + horizon), left).at(x),
    fill: (col, row) => if row == 0 {luma(240)},
    [*name*], [description],
    [`generic_descriptor`], [対象GenericへのDescriptor],
    [`type`], [作成するCapabilityのType],
    [`size`],
    [作成するCapabilityのSize \ #list(
        [Capabilityが固定長の場合, 値は無視される]
    )],
    [`count`], [作成するCapabilityの個数],
    [`node_descriptor`], [格納先NodeへのDescriptor],
    [`node_depth`], [格納先Nodeを探索する深さ],
    [`node_index`], [格納先NodeのSlot],
)
