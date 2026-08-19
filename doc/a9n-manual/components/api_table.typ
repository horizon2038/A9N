#import "/components/fonts.typ" : HEADING_FONT

#let api_table(..args) = {
    let values = args.pos().flatten()
    let row_count = 1 + values.len() / 3
    table(
        stroke: (x, y) => (
            top: if y == 0 { 0.8pt } else { 0pt },
            bottom: if y == 0 { 0.45pt }
                else if y == row_count - 1 { 0.8pt }
                else { 0pt },
        ),
        gutter: 0.4em,
        columns: 3,
        inset: (
            x: 1.5em,
            y: 1em,
        ),
        align: (x, y) => ((left + horizon), (left + horizon), left).at(x),

        table.header(
            text(font: HEADING_FONT, weight: "medium")[type],
            text(font: HEADING_FONT, weight: "medium")[name],
            text(font: HEADING_FONT, weight: "medium")[description],
        ),
        ..values
            .enumerate()
            .map(element => {
                let (index, value) = element
                if (calc.rem(index, 3) == 0 or calc.rem(index + 2, 3) == 0) {
                    /* add raw-text */
                    [#raw(value)]
                }
                else {
                    [#value]
                }
            })
    )
}

#let normal_table(..args) = {
    let values = args.pos().flatten()
    let row_count = 1 + values.len() / 2
    table(
        stroke: (x, y) => (
            top: if y == 0 { 0.8pt } else { 0pt },
            bottom: if y == 0 { 0.45pt }
                else if y == row_count - 1 { 0.8pt }
                else { 0pt },
        ),
        gutter: 0.4em,
        columns: 2,
        inset: (
            x: 1.5em,
            y: 1em,
        ),
        align: (x, y) => ((left + horizon), (left + horizon), left).at(x),

        table.header(
            text(font: HEADING_FONT, weight: "medium")[name],
            text(font: HEADING_FONT, weight: "medium")[description],
        ),
        ..values
            .enumerate()
            .map(element => {
                let (index, value) = element
                if (calc.rem(index, 2) == 0) {
                    /* add raw-text */
                    [#raw(value)]
                }
                else {
                    [#value]
                }
            })
    )
}
