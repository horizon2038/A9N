#let api_table(..args) = {
    table(
        columns: 2,
        inset: (
            x: 1.5em,
            y: 1em,
        ),
        align: (x, y) => ((left + horizon), left).at(x),

        fill: (col, row) => if row == 0 {luma(240)},
        [*name*],
        [*description*],
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
