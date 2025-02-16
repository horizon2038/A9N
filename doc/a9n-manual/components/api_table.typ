#let api_table(..args) = {
    table(
        stroke: (x, y) => (
            bottom: if y > 0 {
                0pt 
            } else {
                0.5pt
            },
        ),
        gutter: 0.4em,
        columns: 3,
        inset: (
            x: 1.5em,
            y: 1em,
        ),
        align: (x, y) => ((left + horizon), (left + horizon), left).at(x),

        fill: (col, row) => if row == 0 {
            luma(360)
        },
        table.header(
            [*type*],
            [*name*],
            [*description*],
        ),
        ..args
            .pos()
            .flatten()
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
    table(
        stroke: (x, y) => (
            bottom: if y > 0 {
                0pt 
            } else {
                0.5pt
            },
        ),
        gutter: 0.4em,
        columns: 2,
        inset: (
            x: 1.5em,
            y: 1em,
        ),
        align: (x, y) => ((left + horizon), (left + horizon), left).at(x),

        fill: (col, row) => if row == 0 {
            luma(360)
        },
        table.header(
            [*name*],
            [*description*],
        ),
        ..args
            .pos()
            .flatten()
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
