#import "layout.typ" : template

#show: doc => template(
    [A9N Manual],
    doc,
)

#align(center, text(20pt)[
    #v(1fr)
    *A9N Manual* \
    #text(10pt)[Version 0.0.1]
    #v(2fr)
])
