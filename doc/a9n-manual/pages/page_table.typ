#import "@preview/bytefield:0.0.6": *
#import "/components/api_table.typ" : *

= Page Table

== Introduction

== Page Table API

=== `map`

#api_table(
    "descriptor", "page_table_descriptor", "対象Page TableへのDescriptor",
    "descriptor", "target_descriptor", "対象にMapするDescriptor (Page Table or Frame)",
    "virtual_address", "address", "Mapする仮想アドレス",
    "flags", "flags", "Mapに使用するFlag",
)

=== `unmap`

#api_table(
    "descriptor", "page_table_descriptor", "対象Page TableへのDescriptor",
    "descriptor", "target_descriptor", "UnmapするPage TableへのDescriptor",
    "virtual_address", "address", "Unmapする仮想アドレス",
)
