= Kernel Call

== Introduction

A9N Microkernelは3タイプのKernel Callを提供します. 言い換えれば, たった3つのKernel Callを使用することでSystem全体を制御することが可能です.
呼び出しにはArchitecture固有の ABI (cf., @ABI) を使用します.

== Capability Call

第一引数としてCapability Descriptorを受け取り, そのCapabilityに対して操作を実行するKernel Callです. 
すべての特権的操作はCapability Callを介して行われます.

\

Capability Callは以下のようなInterface(Pseudo Code)によって実行されます :

```cpp
capability_call(
    target_descriptor,
    operation_type,
    args ...
)
```

Capabilityへの操作は低レベル呼び出しのWrapperとして実装されます. `operation_type`はLibrary関数自体が設定するため, 呼び出し時に省略されます.


=== Virtual Message Register

Capability CallはVirtual Message Registerを介して行われます. 一定数のMessageはArchitecture固有のRegisterにMappingされ, その数を超えるMessageは実行中のProcess Control Blockに割り当てられた*IPC Buffer*#footnote[L4 Microkernel FamilyにおけるUTCBのようなもの]に格納されます.
Map可能なRegister数を超えない限り, Capability Callはコピーなしに実行可能です.

== Yield

*Yield*は実行中のContextをBlockし, Time Sliceを他のContextへ移譲するためのKernel Callです.
Capabiltiy Callとは異なり, すべての実行中Contextで使用可能です.

== Debug Call

Debug CallはDebugに使用するためのKernel Callです. 
Capabilityを必要としませんが, Production Projectにおける使用は推奨されません.
