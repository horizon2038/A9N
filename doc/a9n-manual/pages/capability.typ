#import "@preview/bytefield:0.0.6": *
#import "/components/api_table.typ" : *

= Capability

== Introduction

== Rights

=== `capability_rights`

#bytefield(
    bpr: 8,
    rows: (4em),
    bitheader(
        "bounds",
        0,
        8,
        text-size: 8pt,
    ),

    flag[READ],
    flag[WRITE],
    flag[COPY],
    bits(5)[RESERVED],

    text-size: 4pt,
)

== Dependency Node
