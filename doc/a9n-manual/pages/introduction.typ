#import "layout.typ" : template

#show: doc => template(
    [introduction],
    doc,
)

= Introduction

== about A9N

A9Nは, HALを用いて移植容易性を実現する, Capability-Based Microkernelです.
ユーザーからのカーネルに対するすべての操作は, Capabilityと呼ばれるアクセス権を用いて実行されます.
基本的に, ユーザーからのカーネル呼び出しであるシステムコールは`ipc()`メカニズムを用いて,

```cpp
ipc(capability_descriptor, args ... )
```

のように行われます.

しかしながら, この`ipc()`を直接呼び出す形式は最も低レベルなものであるため, `liba9n`ライブラリを用いた呼び出しを使用することが推奨されます.

例えば, Generic Capabilityに対するConvert操作には, 
```cpp
common::error convert(
    generic_descriptor,
    type,
    size,
    count,
    node_descriptor,
    node_depth,
    node_index
)
```
のようなライブラリ関数が用意されます.

同様に, 他すべてのCapabilityに対する操作へ, ラッパーであるライブラリ関数が用意されます.
