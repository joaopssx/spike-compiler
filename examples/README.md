# Examples

Small Portugol programs. Each one shows a single idea.

| File | What it shows |
| --- | --- |
| `hello.por` | The smallest program: write a message. |
| `sum.por` | Reading two numbers with `leia` and writing their sum. |
| `average.por` | Reading several values and using `real` division. |
| `operators.por` | `div`, `mod` and `^`. |
| `logic.por` | `e`, `ou`, `nao` and comparisons in conditions. |
| `nested_if.por` | A `se` inside another `se`. |
| `menu.por` | A calculator menu with `escolha`. |
| `countdown.por` | An `enquanto` loop counting from 10 down to 1. |
| `repeat_until.por` | A `repita ... ate` loop that runs at least once. |
| `times_table.por` | A `para` loop with `passo`. |
| `break.por` | Leaving a loop early with `interrompa`. |
| `constants.por` | A `const` section. |
| `vector_sum.por` | A vector filled with `leia` and walked with `para`. |
| `vector_max.por` | Finding the largest value of a vector and where it is. |
| `matrix.por` | Multiplying two matrices. |
| `bubble_sort.por` | Sorting a vector, with a procedure that swaps two elements. |
| `procedures.por` | Procedures with parameters. |
| `swap.por` | Parameters by reference (`var`). |
| `factorial.por`, `fibonacci.por` | Recursive functions. |
| `primes.por` | A function that returns `logico`. |
| `math.por`, `text.por` | Built-in functions for math and text. |
| `palindrome.por` | Working with text, character by character. |

Compile and run any of them:

```
spike examples/hello.por --run
```

Every file here is also a test: CTest runs `spike tokens` on each one and expects `parse: OK`.
