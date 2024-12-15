#let template(title, doc) = {

    show heading.where(level: 1): it => block(width:100%)[
        #set align(left)
        #set text(20pt, weight: "bold")
        #it
    ]

    show heading.where(level: 2): it => block(width:100%)[
        #v(10.5pt)
        #set align(left)
        #set text(15pt, weight: "regular")
        #it
        #v(5pt)
    ]

    set heading(numbering: "1.")
    
    set text(
        font: "FOT-TsukuAOldMin Pr6N",
    )
    
    set page(
        margin: auto,
        header: [
            #set text(10.5pt)
            #smallcaps[a9n manual]
            #h(1fr) #title
        ],
        numbering: "1",
        number-align: center,
    )

    set par(
        leading: 1.24em,
        first-line-indent: 0em,
    )

    show raw: set block(fill: luma(240), inset: 2em, radius: 0.5em, width: 100%)
    show raw: set text(font: "Hack")

    h(1em)
    doc
}
