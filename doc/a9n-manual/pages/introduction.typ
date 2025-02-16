= Introduction

== A9N Microkernel Overview

*A9N Microkernel*は3rd-Generation Capability-Based Microkernelです.

最適化されたIPC機構と*Object-Capability Model*によるセキュリティ機構を持ち, 低Latencyかつ高信頼性なシステムを実現します.
また, 高度に設計された*HAL*によって, さまざまなハードウェアプラットフォームにPortingが可能です.

このマニュアルは, A9N Microkernelの概要とその設計について解説するものです.

== Capability

特権的操作に対する安全性は, *Capability*と呼ばれる偽造不可能かつ譲渡可能なTokenによって保たれます.
Capabilityは従来のACL的なアプローチとは異なり, その所有者がその権限を持つことを示すものです. これにより, より高い粒度の権限管理を実現します.

== IPC
