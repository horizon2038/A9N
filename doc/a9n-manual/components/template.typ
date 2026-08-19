#import "/components/title.typ" : title_block
#import "/components/style.typ"
#import "/components/toc.typ" : toc

#let template(
  title: [],
  version: [],
  author: [],
  date: datetime(year: 2026, month: 1, day: 1),
  keywords: (),
  front_matter: none,
  body,
) = {
  show: style.configure.with()

  set document(
    title: title,
    author: author,
    date: date,
    keywords: keywords,
  )

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

  /* ===== front matter ===== */
  set page(numbering: "1")
  if front_matter != none {
    front_matter
  }

  /* ===== table of contents ===== */
  heading(level: 1, numbering: none, outlined: false)[Table of Contents]
  toc()

  /* ===== main body ===== */
  /* reset page numbering */
  set heading(numbering: "1.1")
  set page(header: style.running_header())
  body

  /* ===== bibliography ===== */
  /* Add a bibliography here after references.bib receives its first entry. */
  bibliography("/resources/references.bib", title: "References")

}
