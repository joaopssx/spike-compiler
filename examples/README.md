# Examples

Small Portugol programs that use only what Spike understands today. Each one shows a single idea.

| File | What it shows |
| --- | --- |
| `hello.por` | The smallest program: write a message. |
| `sum.por` | Reading two numbers with `leia` and writing their sum. |
| `average.por` | Reading several values at once and using `real` division. |
| `countdown.por` | An `enquanto` loop counting from 10 down to 1. |
| `times_table.por` | A `para` loop with `passo`. |
| `repeat_until.por` | A `repita ... ate` loop that runs at least once. |
| `logic.por` | `e`, `ou`, `nao` and comparisons in conditions. |
| `nested_if.por` | A `se` inside the `senao` of another `se`. |

Spike can't generate executables yet. For now you can check that a program is read correctly:

```
build/spike tokens examples/hello.por
```

The output lists the tokens and ends with `parse: OK` when the program is valid.

Every file here is also a test: CTest runs `spike tokens` on each one and expects `parse: OK`.
