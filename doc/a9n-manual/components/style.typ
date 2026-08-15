/* theme */
#let BASE_COLOR = rgb("141414")
#let MAIN_COLOR = rgb("ededed")
// #let ACCENT_COLOR = rgb("0099e6")
#let ACCENT_COLOR = rgb("141414")
#let HEADING_FONT = "A-OTF Gothic MB101 Pr6N"
#let BODY_FONT = "A-OTF Ryumin Pro"

#let configure(body) = {

  /* ===== global settings ===== */
  /* page */
  set page(
    paper: "a4",
    margin: (x: 10em, y: 10em),
    columns: 1,
  )

  /* text */
  set text(
    // font: "A-OTF Ryumin Pro",
    font: "A-OTF Ryumin Pro",
    size: 10pt,
  )
  set strong(delta: 100)
  show strong: set text(weight: "medium")

  /* heading */
  set heading(numbering: "1.a")
  /* default heading style */
  show heading: it => {
    text(font: HEADING_FONT, weight: "medium")[#it]
    v(0.5em)
  }
  show heading.where(level: 1): set text(font: HEADING_FONT, size: 24pt, weight: "bold")
  show heading.where(level: 2): set text(font: HEADING_FONT, size: 16pt, weight: "semibold")
  show heading.where(level: 3): set text(font: HEADING_FONT, size: 12pt, weight: "semibold")
  show heading: set block(sticky: true)

  show heading.where(level: 1): it => {
      let current_chapter_number = counter(heading.where()).at(here()).first()
      colbreak(weak: true)
      block(width:100%)[
          #set align(right)
          #set text(20pt, weight: "bold")
          #v(5%)
          #if it.numbering == none {
            // do nothing
          } else {
            text(40pt)[#current_chapter_number]
          }
          #v(1%)
          #it.body
          #v(5%)
      ]
  }
  show heading.where(level: 2): it => {
    let current_chapter_number = counter(heading.where()).at(here()).at(0, default: none)
    let current_section_number = counter(heading.where()).at(here()).at(1, default: none)

    block(
      stack(
        dir: ltr,
        spacing: 0.5em,
        box(
          rect(
            width: 0.4em,
            height: 0.8em,
            fill: ACCENT_COLOR,
            radius: 0pt,
          ),
          inset: (bottom: -0.1em),
        ),
        [#current_chapter_number.#current_section_number #it.body]
      )
    )
    v(0.5em)
  }
  /*
  show heading.where(level: 3): it => {
  }
  */
  set heading(numbering: "1.")

  /* figure */
  show figure: set block(breakable: true)

  /* paragraph */
  set par(
    first-line-indent: (amount: 1em, all: true),
  )
  
  /* math equation */
  set math.equation(
    numbering: "(1.1)"
  )

  /* bibliography */
  show bibliography : set heading(level: 1)

  /* ===== main body ===== */
  body
}

