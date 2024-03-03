// Definition of chapter outline
#let toc() = {
  align(left)[
    #text(size: 20pt, weight: "bold")[
      #v(30pt)
      Contents
      #v(30pt)
    ]
  ]

  set text(size: 12pt)
  set par(leading: 1.24em, first-line-indent: 0pt)
  locate(loc => {
    let elements = query(heading.where(outlined: true), loc)
    for el in elements {
      let before_toc = query(heading.where(outlined: true).before(loc), loc).find((one) => {one.body == el.body}) != none
      let page_num = if before_toc {
        numbering("i", counter(page).at(el.location()).first())
      } else {
        counter(page).at(el.location()).first()
      }

      link(el.location())[#{
        // acknoledgement has no numbering
        let chapt_num = if el.numbering != none {
          numbering(el.numbering, ..counter(heading).at(el.location()))
        } else {none}

        if el.level == 1 {
          set text(weight: "black")
          if chapt_num == none {} else {
            chapt_num
            "  "
          }
          el.body
        } else if el.level == 2 {
          h(2em)
          chapt_num
          " "
          el.body
        } else {
          h(5em)
          chapt_num
          " "
          el.body
        }
      }]
      box(width: 1fr, h(0.5em) + box(width: 1fr, repeat[.]) + h(0.5em))
      [#page_num]
      linebreak()
    }
  })
}

#import "layout.typ" : template

#show: doc => template(
    [contents],
    doc,
)

#toc()
