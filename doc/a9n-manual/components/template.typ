#import "/components/title.typ" : title_block
#import "/components/style.typ"
#import "/components/toc.typ" : toc

#let template(
  title: [],
  version: [],
  author: [],
  date: datetime(year: 2026, month: 1, day: 1),
  body,
) = {
  show: style.configure.with()

  /* ===== epilogue of the title page ===== */
  /* reset page numbering */
  // set page(numbering: "1")
  set heading(numbering: none)

  /* ===== prologue of the title page ===== */
  title_block(
    title: title,
    version: version,
    author: author,
    date: date,
  )

  v(2em)
  pagebreak()
  counter(page).update(1)

  /* ===== table of contents ===== */
  /* reset page numbering */
  set page(numbering: "1")

  [== Table of Contents]
  toc()

  /* ===== main body ===== */
  /* reset page numbering */
  set heading(numbering: "1.a")
  body

  /* ===== bibliography ===== */
  pagebreak()
  bibliography("/resources/references.bib", title: "References")
}

