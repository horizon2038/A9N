/* Components shared by reference pages. */

#import "/components/fonts.typ" : HEADING_FONT, BODY_FONT

#let term(body) = text(font: HEADING_FONT, weight: "medium", body)

#let reference_table(columns, headers, ..cells) = {
  let body_cells = cells.pos()
  let column_count = columns.len()
  let row_count = 1 + body_cells.len() / column_count

  set text(size: 8pt)
  set par(first-line-indent: 0pt)

  let rendered = table(
    columns: columns,
    inset: (x: 0.6em, y: 0.5em),
    gutter: 0pt,
    stroke: (column, row) => (
      top: if row == 0 { 0.8pt + black } else { 0pt },
      bottom: if row == 0 { 0.45pt + black }
        else if row == row_count - 1 { 0.8pt + black }
        else { 0pt },
    ),
    table.header(..headers.map(header => term(header))),
    ..body_cells,
  )

  // Keep short three-line tables on one page. Long reference tables remain
  // breakable so that inventories and field lists do not overflow the page.
  if row_count <= 4 {
    block(width: 100%, breakable: false, rendered)
  } else {
    rendered
  }
}

#let fields(columns: (1fr, 2.4fr), ..cells) = reference_table(
  columns,
  ([項目], [説明]),
  ..cells,
)

#let operations(..cells) = {
  let values = cells.pos()

  assert(
    calc.rem(values.len(), 4) == 0,
    message: "operations requires four cells for each method",
  )

  for index in range(values.len()) {
    if calc.rem(index, 4) == 0 {
      block(
        width: 100%,
        breakable: false,
        below: 0.9em,
      )[
        #reference_table(
          (2.3fr, 1.2fr, 2.6fr, 1.2fr),
          ([Method], [Message Register], [Description], [Error]),
          values.at(index),
          values.at(index + 1),
          values.at(index + 2),
          values.at(index + 3),
        )
      ]
    }
  }
}

#let status_table(..cells) = reference_table(
  (1.2fr, 1.7fr, 0.9fr, 2.4fr),
  ([Object], [Type], [実装状態], [作成方法と役割]),
  ..cells,
)

#let notice(kind, body) = {
  block(
    width: 100%,
    breakable: false,
    inset: 0.8em,
    stroke: 0.6pt + rgb("9a9a9a"),
    fill: rgb("f5f5f5"),
    radius: 1pt,
  )[
    #set par(first-line-indent: 0pt)
    #text(font: HEADING_FONT, weight: "semibold")[#kind]
    #linebreak()
    #body
  ]
}
