#import "/components/fonts.typ" : HEADING_FONT, BODY_FONT

// Definition of chapter outline
#let toc() = {
  set text(font: HEADING_FONT, size: 10.5pt)
  set par(leading: 1.12em, first-line-indent: 0pt)
  context {
    let loc = here()
    let elements = query(heading.where(outlined: true))
    // chapter-start markers are emitted for every level-1 heading, including
    // an unoutlined Table of Contents heading. Use the same population here
    // so that each outline entry maps to its own physical chapter page.
    let chapters = query(heading.where(level: 1))
    let chapter_starts = query(<chapter-start>)

    for el in elements {
      let before_toc = query(heading.where(outlined: true).before(loc)).find((one) => {one.body == el.body}) != none
      let target = if el.level == 1 {
        let index = chapters.position((one) => one.location() == el.location())
        if index != none and index < chapter_starts.len() {
          chapter_starts.at(index).location()
        } else {
          el.location()
        }
      } else {
        el.location()
      }

      let page_num = if before_toc {
        numbering("1", counter(page).at(target).first())
      } else if (el.level >= 1) and (el.level <= 3) {
        counter(page).at(target).first()
      } else {
      }

      link(target)[#{
        // acknoledgement has no numbering
        let chapt_num = if el.numbering != none {
          numbering(el.numbering, ..counter(heading).at(el.location()))
        } else {none}

        if el.level == 1 {
          set text(weight: "semibold")
          if chapt_num == none {} else {
            chapt_num
            "  "
          }
          el.body
        } else if el.level == 2 {
          h(1em)
          chapt_num
          " "
          el.body
        } else if el.level == 3 {
          h(2em)
          chapt_num
          " "
          el.body
        } else {
            continue
        }
      }]
      box(width: 1fr, h(0.5em) + box(width: 1fr, repeat[.]) + h(0.5em))
      [#page_num]
      linebreak()
    }
  }
}
