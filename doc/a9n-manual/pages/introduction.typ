= Introduction

== A9N Microkernel

*A9N Microkernel*は第3世代のCapability-Based Microkernelです.

高速なIPC機構とObject-Capability Modelに基づくセキュリティ機構を備えており，低レイテンシかつ高信頼性のシステムを実現します．また，アーキテクチャ依存の箇所はHAL（ハードウェア抽象化レイヤ）によって抽象化されており，さまざまなハードウェアプラットフォームへの移植が容易です．

このマニュアルは, A9N Microkernelの概要とその設計について解説するものです.

== Microkernel Architecture

== IPC Mechanism

== Capability

A9Nにおける特権的な操作はカーネルオブジェクト呼び出しという形に抽象化され，その操作は*Capability*という偽造不可能かつ譲渡可能なトークンによって保護されます．Capabilityは従来のACL（アクセス制御リスト）による認証とは異なり，「Capabilityを所有していること」そのものがオブジェクトに対する操作権限を示します．

// figure: ACL vs Capability

A9N上のユーザーレベルシステムは，Capabilityをキーとして`capability_call(capability, operation, args...)`という形でカーネルオブジェクトに対する操作を実行可能です．したがって，A9Nはオブジェクト指向プログラミングと同等のインターフェースを持つこととなります．

== Kernel Object

A9Nにおけるカーネルオブジェクトは，Capabilityを介して操作可能なシステムリソースの抽象化です．カーネルオブジェクトは，ユーザーレベルのシステムが必要とするリソースを表現するために設計されており，その種類は以下の通りです．

=== Capability Node

Capability NodeはCapabilityを格納して管理するためのカーネルオブジェクトです．Capability Nodeは$2^n$個のCapability Slotを持ち，それぞれのSlotへCapabilityを格納・移動・コピー・削除が可能です．また，Capability Node自体もCapabilityであるため，Capability NodeをCapabilityとして他のCapability Nodeに格納することも可能です．

=== Generic

Genericは，カーネルオブジェクトを作成するための抽象メモリ領域です．

=== Process Control Block

=== IPC Port

=== Notification Port

=== Interrupt Region

=== Interrupt Port

=== I/O Port

=== Address Space

=== Page Table 

=== Frame 

=== Virtual CPU

=== Virtual Address Space

=== Virtual Page Table
