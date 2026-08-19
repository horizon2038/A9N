/* theme */
#let BASE_COLOR = rgb("141414")
#let MAIN_COLOR = rgb("ededed")
#let ACCENT_COLOR = rgb("141414")

#import "/components/fonts.typ" : HEADING_FONT, BODY_FONT

#let running_header() = context {
  let physical_page = here().page()
  let chapters = query(heading.where(level: 1))
  let sections = query(heading.where(level: 2))
  let chapter_origins = query(<chapter-origin>)
  let chapter_starts = query(<chapter-start>)
  let pair_count = calc.min(chapter_origins.len(), chapter_starts.len())
  let blank_page = range(pair_count).any(index => {
    let heading_location = chapter_origins.at(index).location()
    let start_location = chapter_starts.at(index).location()
    let heading_page = heading_location.page()
    let start_page = start_location.page()
    let skipped_page = physical_page > heading_page and physical_page < start_page
    let empty_origin_page = (
      physical_page == heading_page
      and heading_page < start_page
      and heading_location.position().y <= start_location.position().y
    )
    skipped_page or empty_origin_page
  })

  if not blank_page {
    let current_chapter_index = range(pair_count)
      .filter(index => chapter_starts.at(index).location().page() <= physical_page)
      .at(-1, default: none)
    let current_chapter = if current_chapter_index == none {
      none
    } else {
      chapters.at(current_chapter_index)
    }
    let current = if current_chapter == none {
      none
    } else if calc.odd(physical_page) {
      let chapter_start_location = chapter_starts.at(current_chapter_index).location()
      let chapter_start_page = chapter_start_location.page()
      let chapter_sections = sections.filter(section => {
        let page = section.location().page()
        page >= chapter_start_page and page <= physical_page
      })
      let on_page = chapter_sections.filter(
        section => section.location().page() == physical_page,
      )
      let begins_near_top = (
        on_page.len() > 0
        and on_page.at(0).location().position().y
          <= chapter_start_location.position().y + 120pt
      )
      if on_page.len() > 0 and (
        physical_page == chapter_start_page or begins_near_top
      ) {
        on_page.at(0)
      } else {
        chapter_sections
          .filter(section => section.location().page() < physical_page)
          .at(-1, default: current_chapter)
      }
    } else {
      current_chapter
    }

    if current != none {
      block(
        width: 100%,
        inset: (bottom: 0.5em),
        stroke: (bottom: 0.5pt + BASE_COLOR),
      )[
        #set text(font: HEADING_FONT, size: 8pt, weight: "medium")
        #if calc.odd(physical_page) {
          align(right, current.body)
        } else {
          align(left, current.body)
        }
      ]
    }
  }
}

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
    font: BODY_FONT,
    size: 10pt,
  )
  set strong(delta: 100)
  show strong: set text(weight: "medium")

  /* heading */
  set heading(numbering: "1.1")
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
      [#metadata(none) <chapter-origin>]
      pagebreak(to: "odd")
      [#metadata(none) <chapter-start>]
      let current_chapter_number = counter(heading.where()).at(here()).first()
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
    let heading_body = if it.numbering == none {
      it.body
    } else {
      [#current_chapter_number.#current_section_number #it.body]
    }

    block(
      breakable: false,
      sticky: true,
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
        heading_body,
      )
    )
    v(0.5em)
  }
  /*
  show heading.where(level: 3): it => {
  }
  */
  // set heading(numbering: "1.")

  /* figure */
  show figure: set block(breakable: false)

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

  /* link */
  show link: it => {
    underline(offset: 2pt)[#it]
  }

  /* ===== main body ===== */
  body
}
