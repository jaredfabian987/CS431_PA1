# CS 433 PA1 — starter kit

Everything here already builds and runs. Start by proving that:

```
make
./ptime /bin/echo hello
```

You will see a report with the right shape and all-zero numbers, because no
child process exists yet. Your job is in `ptime.c`, at the five `TODO` markers.

## What is in this kit

| File | What it is |
|---|---|
| `PA1_instructions.md` | The assignment handout. Read it first; it is the specification. |
| `ptime.c` | The one file you edit. Five TODOs in `main()`. |
| `Makefile` | Builds `ptime` and the workload programs. `make test` runs the self-check. |
| `analysis.md` | The five written questions. Fill it in; it is 20 of the 100 points. |
| `README.md` | This file. **You replace it** — see "Before you submit" below. |
| `workload/spin.c` | Burns CPU without sleeping. Tests your **user CPU** number. |
| `workload/sleeper.c` | Sleeps without using CPU. Tests your **wall clock** number. |
| `workload/statuser.c` | Exits with a chosen code, or kills itself with a chosen signal. Tests your **status** reporting. |
| `workload/noisy.c` | Writes one line to stdout and one to stderr. Tests that your report stays on **stderr**. |
| `workload/buffer_trap.c` | The demonstration for question 1. Run it two ways. |
| `tests/` | The 14 self-check tests. `tests/README.md` says what each one catches. |

You do not need to modify anything in `workload/` or in `tests/`. You do not
need to modify `print_report()` — it already emits the exact format the tests
compare against, and changing it will cost you points rather than earn them.

The graders run the tests from their own fresh copy of `tests/`, so editing
yours changes nothing except your ability to find your own bugs.

## Before you submit

```
make clean && make          # must produce ZERO warnings
make test                   # must print "all tests passed"
make clean                  # then tar — no binaries in the submission (PA1_instructions.md §9)
```

Three things students forget every term:

1. Put your name(s) on the header line at the top of `ptime.c`.
2. Put your name(s) in the header of `analysis.md`.
3. **Replace this file.** `README.md` in your submission is *yours*: your
   name(s), how to build your program, and anything the grader should know —
   including any source you cited. Delete everything above this section and
   write your own. Leaving the starter's README in place is worth 0 on the
   authorship line of the rubric.

`PA1_instructions.md` §9 has the exact tarball name and layout.

## If `make test` cannot find the tests

The kit ships `tests/` next to this `Makefile`. If yours is missing, download
the kit again from Canvas rather than working without it.
