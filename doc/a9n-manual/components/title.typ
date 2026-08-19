#import "/components/fonts.typ" : HEADING_FONT, BODY_FONT

#let __title_internal(
  title: [],
  version: [],
  author: [],
  date: datetime(year: 2026, month: 1, day: 1),
) = {
      block[#text(font: HEADING_FONT, size: 20pt)[
          #v(1fr)
          *#title* \
          #text(14pt, weight: "medium")[v#version] \
          #v(0.5fr)
          #text(14pt)[#author] \
          #text(14pt)[#date.year()-#date.month()-#date.day()] \
          #v(2fr)
        ]]
}

#let title_block(
  title: [],
  version: [],
  author: [],
  date: datetime(year: 2026, month: 1, day: 1),
) = {
  place(
      top + center,
      float: true,
      scope: "parent",
      __title_internal(
          title: title,
          version: version,
          author: author,
          date: date,
      )
  )
}

