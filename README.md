# CS 433 PA1 — ptime

Everything here builds and runs with:

```
make
./ptime /bin/echo hello

```

The program creates a child process, runs the requested command, waits for it to finish, and reports the child process status along with wall time, user CPU time, and system CPU time.

## What is in this kit

| File | What it is |
|---|---|
| `PA1_instructions.md` | The assignment handout and specification. |
| `ptime.c` | Our completed implementation of `ptime`. |
| `Makefile` | Builds `ptime` and the workload programs. `make test` runs the self-check. |
| `analysis.md` | Our completed answers to the five written analysis questions. |
| `README.md` | This file. |
| `workload/spin.c` | Burns CPU without sleeping and tests user CPU time. |
| `workload/sleeper.c` | Sleeps without using much CPU and tests wall-clock time. |
| `workload/statuser.c` | Exits with a chosen code or signal and tests status reporting. |
| `workload/noisy.c` | Writes to stdout and stderr and tests that the report stays on stderr. |
| `workload/buffer_trap.c` | Demonstrates the buffering issue used in analysis question 1. |
| `tests/` | The 14 provided self-check tests. |

We did not modify anything in `workload/` or `tests/`. We also did not modify the provided `print_report()` helper.

## Before you submit

```
make clean && make
make test
make clean
```

Names:

Kiernan Flieh  
Cyril Tabaranza  
Nathan Nguyen  
Jared Fabian  
Nick Pieratos

Built on Ubuntu through WSL using GCC 13.3.0 and tested on the CSUSM CS433 course server using GCC 11.5.0.

The program uses `fork()`, `execvp()`, `waitpid()`, `clock_gettime(CLOCK_MONOTONIC, ...)`, and `getrusage(RUSAGE_CHILDREN, ...)`.

The program reports its timing information to stderr and returns the child process's exit code, or `128 + signal number` if the child is terminated by a signal.

Sources cited: None.

## If `make test` cannot find the tests

The `tests/` directory should be located next to the `Makefile`. Run the provided test suite with:

``` make test  ```

