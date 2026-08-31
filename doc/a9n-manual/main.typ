/* a9n manual entry point */

#import "/components/template.typ" : template
#import "@preview/cjk-spacer:0.2.1": cjk-spacer
#import "/pages/document_status.typ" : document_status

#let manual_part(number, title, description) = {
  heading(
    level: 1,
    numbering: none,
    outlined: true,
  )[#number  #title]
  description
}

#show: cjk-spacer
#show: template.with(
    title: "A9N Microkernel Manual",
    version: read("version.txt"),
    author: "Rekka 'horizon' IGUMI",
    date: datetime(year: 2026, month: 8, day: 31),
    keywords: (
      "A9N",
      "microkernel",
      "capability-based security",
      "symmetric multiprocessing",
      "operating system",
    ),
    front_matter: document_status(read("version.txt")),
)

#manual_part([Part I], [Start Here], [本Partは，A9N Microkernelの対象範囲と，標準構成を起動する最短の手順を示す．])
#include "/pages/introduction.typ"
#include "/pages/getting_started.typ"

#manual_part([Part II], [Architecture-independent Kernel Interface], [本Partは，すべてのArchitectureで共有するCapability ModelとKernel Call Interfaceを定義する．])
#include "/pages/capability.typ"
#include "/pages/kernel_call.typ"

#manual_part([Part III], [Kernel Object Reference], [本Partは，Architectureに依存しないKernel ObjectをTypeごとに記載する．])
#include "/pages/node.typ"
#include "/pages/generic.typ"
#include "/pages/memory.typ"
#include "/pages/process_control_block.typ"
#include "/pages/ipc_port.typ"
#include "/pages/notification_port.typ"
#include "/pages/interrupt.typ"
#include "/pages/io_port.typ"

#manual_part([Part IV], [System Construction], [本Partは，Boot後のInitとUser-level Systemを構成する方法を説明する．])
#include "/pages/init.typ"
#include "/pages/software_development.typ"
#include "/pages/building_services.typ"

#manual_part([Part V], [Multiprocessing and Architecture-specific Interface], [本Partは，SMPの実行Model，x86_64とAArch64固有のABI，新しいArchitectureとPlatformへHALを移植する手順を示す．])
#include "/pages/smp.typ"
#include "/pages/abi.typ"
#include "/pages/aarch64_abi.typ"
#include "/pages/porting.typ"

#manual_part([Part VI], [Ecosystem], [本Partは，A9N Microkernelと組み合わせるSoftware Componentの責務と選択基準を示す．])
#include "/pages/ecosystem.typ"

#manual_part([Back Matter], [Glossary and References], [本Partは，用語集，謝辞，参考文献を収録する．])
#include "/pages/glossary.typ"
#include "/pages/acknowledgements.typ"
