#import "/components/layout.typ" : template

#show: doc => template(
    [introduction],
    doc,
)

= Introduction

== A9N Microkernel Overview

*A9N Microkernel*とは, 
A9NはHALを用いて移植容易性を実現するCapability-Based Microkernelです.

== Capability

A9N KernelはObject-Capability Modelによる強固なセキュリティ機構を実現します.
*Capability*とは, 偽造不可能かつ譲渡可能なTokenです.
Userによる特権的呼び出しはKernel ObjectへのCapabilityを介した操作としてモデル化されます.

== Capabilityの使用

基本的に, Userからの特権的呼び出しであるCapability Callは`capability_call()`メカニズムを用いて,

```cpp
capability_call(capability_descriptor, ... )
```

のように行われます.

しかしながら, この`capability_call()`は最もPrimitiveなAPIであるため, 実際の使用にはそれらをラップする`liba9n`ライブラリを使用することが推奨されます.

例えば, Generic Capabilityに対するConvert操作には, 
```cpp
generic_result<> convert(
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

同様に, 他すべてのCapabilityに対する操作へライブラリ関数が用意されます.
