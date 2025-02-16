#import "@preview/bytefield:0.0.6": *
#import "/components/api_table.typ" : *

= IPC Port <ipc_port>

== Introduction

IPC PortはIPCを行うためのCapabilityです.
SenderとReceiverのQueueを持ち, 1:nもしくはn:1の通信を可能とします.

`send`や`receive`などの一般的な操作に加え,
Usecaseに最適化された`call`, `reply`, `reply_receive`などのAPIも提供されます.

=== Reply Mechanism

TODO: -

== IPC Port Data Structure

=== `message_info`

#bytefield(
    bpr: 16,
    rows: (8em),
    bitheader(
        "bounds",
        0,
        8,
        15,
        text-size: 8pt,
    ),

    flag[BLOCK],
    bytes(1)[MESSAGE_LENGTH],
    bits(6)[TRANSFER_COUNT],
    flag[KERNEL],
    text-size: 4pt,
)

#normal_table(
    "BLOCK", "設定されている場合, IPC操作はBlockされます",
    "MESSAGE_LENGTH", "メッセージの長さ (WORD_BITS単位)",
    "TRANSFER_COUNT", "TransferするCapabilityの数",
    "KERNEL", "設定されていた場合, MessageはKernelからのものであることを示します. UserからこのFlagを設定しても無視されます",
)

== IPC Port API

=== `send`

Messageを送信します. IPC Portが送信待ち状態(Receiver Queueが構成されている)の場合, QueueからProcessを1つ取り出しMessageを送信します.

==== Arguments

#api_table(
    "descriptor", "ipc_port_descriptor", "対象IPC PortへのDescriptor",
    "message_info", "info", "送信するMessageの情報",
)

==== Return Value

#api_table(
    "ipc_port_result", "result", "Messageの送信結果",
)

=== `receive`

Messageを受信します. IPC Portが受信待ち状態(Sender Queueが構成されている)の場合, QueueからProcessを1つ取り出しMessageを受信します.

#api_table(
    "descriptor", "ipc_port_descriptor", "対象IPC PortへのDescriptor",
    "word&", "identifer", "送信元のIdentifier",
)

=== `call`

Messageを送信しReplyを受信する過程を1つのAtomicな操作で実行します.
Reply Mechanismにより, 送信したMessageを受信したProcessからのReplyを受信することが保証されます.
Message Infoに`BLOCK` Flagが設定されていない場合でも, 対象にReply Objectが設定されていない場合はBlockされます.

#api_table(
    "descriptor", "ipc_port_descriptor", "対象IPC PortへのDescriptor",
    "message_info", "info", "送信するMessageの情報",
)

=== `reply`

Reply Objectが設定されている場合, そのObjectを介して該当ProcessにMessageを送信します.

#api_table(
    "descriptor", "ipc_port_descriptor", [対象IPC PortへのDescriptor#footnote[宛先のDescriptorは本来必要としないが, 今後の拡張を考慮し引数として受け取る]],
)

=== `reply_receive`

Replyを実行してからMessageを受信する過程を1つのAtomicな操作で実行します.
典型的には*スタートアップ*としてReceiveを実行し, Reply Objectを設定してからループ内でReply Receiveを実行することで高速に通信を行うことが可能です.

#api_table(
    "descriptor", "ipc_port_descriptor", "対象IPC PortへのDescriptor",
    "word&", "identifer", "送信元のIdentifier",
)

=== `identify`

IPC PortにIdentiferを付与します. このIdentiferはCapability Slot Localであり, 実体として同じIPC Portを指していても異なるIdentiferを持つことが可能です.

#api_table(
    "descriptor", "ipc_port_descriptor", "対象IPC PortへのDescriptor",
    "word", "identifier", "IPC Portに付与するIdentifier",
)
