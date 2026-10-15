# Product decisions

Decisions that shape what Spike does. Each one was answered by the author; none was left to the default.

| # | Question | Answer | Date |
| --- | --- | --- | --- |
| 1 | Main audience | Students in their first programming course | 2026-10-13 |
| 2 | Portugol dialect | VisuAlg style, with Spike's additions documented in `language.md` | 2026-10-13 |
| 3 | C compiler on Windows | Ship one with Spike | 2026-10-13 |
| 4 | Supported systems | Windows, Linux and macOS at the same level | 2026-10-13 |
| 5 | Numbers and division | 64-bit `inteiro`; `/` always gives `real`; `div` is integer division | 2026-10-13 |
| 6 | Case in names | Names ignore case, like keywords | 2026-10-13 |
| 7 | Accents in names | ASCII only; `média` is an error | 2026-10-13 |

## What decisions 3 and 4 change in the plan

The original plan assumed the opposite on both.

- **Decision 3.** The plan required an installed compiler in v0.2 and downloaded one with `spike setup` in v0.4. Now the Spike package will carry a C compiler (v0.4); until then one has to be installed. The C compiler driver looks for the bundled compiler first and falls back to one on the `PATH` during development. In v0.4, instead of downloading a compiler, Spike is packaged with one. Which compiler to bundle, and its license, is decided then.
- **Decision 4.** Linux and macOS are no longer "it works". CI builds and tests on all three systems, and each release produces an installable package for each one.
