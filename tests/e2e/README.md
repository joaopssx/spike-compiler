# End-to-end tests

Each folder here is one test case. The runner compiles `program.por` with the real `spike`, runs the executable with a known input and compares what it prints with what you wrote down. If a student would see something different, the test fails.

## Adding a case

Create a folder with a short name in English, for example `tests/e2e/average_of_four/`, and put these files in it:

| File | Required | What it holds |
| --- | --- | --- |
| `program.por` | yes | The program. |
| `expected_out.txt` | yes | Everything the program writes to stdout, byte for byte. Prompts written with `escreva` are included; what the user types is not. |
| `input.txt` | no | What is sent to stdin, one value per line. Without it the program gets an empty input, so a `leia` fails with R007 instead of waiting forever. |
| `expected_code.txt` | no | The expected exit code. Defaults to `0`; runtime errors exit with `101`. |
| `expected_err.txt` | no | Lines that must appear in stderr, in this order. Other lines may appear between them. |
| `time_limit_seconds.txt` | no | Fails the case if running the program (not compiling it) takes longer. |

Then reconfigure (`cmake -S . -B build`). The case shows up as `e2e_<folder>` in CTest. There is nothing to register.

Work the expected output out by hand from the program, not by running Spike and copying what it printed. A case copied from the compiler's own output only proves that the compiler agrees with itself.

## Compile errors

Programs that must not compile go in `tests/e2e_errors/<name>/`, with `program.por`, `expected_diag.txt` and optionally `expected_code.txt` (defaults to `1`). `expected_diag.txt` lists lines that must appear in stderr, in order, with colors removed. Include the source line and the caret line, so the test also checks that the arrow points at the right column:

```
erro[E700]: variável 'y' não foi declarada
   5 |   x <- y + 1
     |        ^
```

Leave out the ` --> file:line:col` line: it contains the absolute path of the case, which changes from machine to machine.

## Running

```
ctest --test-dir build -L e2e --output-on-failure
```

On a failure the runner prints the first line that differs, then the full stdout and stderr.

Line endings never matter: `\r\n` and `\n` compare equal. CTest sets `SPIKE_CC` to the C compiler CMake found. If Spike cannot find a C compiler at all, the runner exits with 77 and CTest reports the case as skipped instead of failed.

## The runner

`runner/` holds `spike_e2e`, a small C++ program built with the project. It uses the same `run_process` as the compiler, so it behaves the same on Linux, macOS and Windows and never goes through a shell.
