#let title_block(title_name: "excellent_title", author: "excellent_author", version: "1.0.0") = {
    align(center, text(20pt)[
        #v(1fr)
        *#title_name* \
        #text(10pt)[Version #version]
        #v(0.5fr)
        #text(10pt)[written by #author]

        #v(2fr)
    ])
}
