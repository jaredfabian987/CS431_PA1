# PA1 Written Analysis

**Name(s):** _(every member's name if you worked in a group)_
**Date:**
**Built and tested on:** _(course server, WSL, or VM — say which, and the output of `gcc --version`)_

Answer all five questions, **including every lettered part** — a question with
(a), (b) and (c) is not answered until all three are. Two to four sentences per
part is the right size; the whole file should come to two or three pages, not
one and not eight. Paste real output from your own runs where a question asks
for it; invented output is easy to spot and is handled as an academic-honesty
issue, not as a wrong answer.

Budget about two hours for this, and write it while the code is fresh in your
head. It is 20 points, which is more than any two tests are worth.

Write your own explanation. "It gets buffered" is not an explanation. Say
*what* is buffered, *where* that buffer lives, and *what happens to it*.

---

## Q1. The buffering trap (4 points)

Build and run the provided demonstration two different ways:

```
make
./workload/buffer_trap
./workload/buffer_trap | cat
```

Run the first command in a real terminal (an ssh session or a WSL shell), not from an editor's Run button. If both outputs look the same, your stdout is not a terminal.

**(a)** Paste both outputs exactly as you got them.

**(b)** They are different, and the program did not change. Explain why, in
terms of where `printf` actually writes and what `fork()` copies.

**(c)** Name the one library call that fixes it, say exactly where it goes in
`ptime.c`, and explain why placing it *after* the `fork()` would not work.

_Your answer:_


---

## Q2. Wall time is not CPU time (4 points)

Run both of these with your finished `ptime` and paste both reports:

```
./ptime ./workload/sleeper 1000
./ptime ./workload/spin 1000
```

**(a)** For each one, say which is larger — wall, or user+sys — and why.

**(b)** A program can finish with `user + sys` *greater* than `wall`. Describe
a program that would do that. (None of the provided workload programs on its own will do it; say what
kind of program would, and what hardware makes it possible. You can check your answer with
`./ptime sh -c "./workload/spin 500 & ./workload/spin 500 & wait"`.)

_Your answer:_


---

## Q3. Which clock, and why (4 points)

You used `CLOCK_MONOTONIC`. `CLOCK_REALTIME` also exists, and it reports the
wall-clock time of day.

**(a)** Describe a concrete situation in which measuring an interval with
`CLOCK_REALTIME` would give a wrong answer — including one where the measured
duration comes out *negative*.

**(b)** Given that, why does `CLOCK_REALTIME` exist at all? Name one job it is
right for and `CLOCK_MONOTONIC` is wrong for.

_Your answer:_


---

## Q4. `_exit` versus `exit` in the child (4 points)

Your child process calls `_exit()` after a failed `execvp()` — never `exit()`,
and never `return`.

**(a)** What does `exit()` do that `_exit()` does not?

**(b)** Connect this to Q1. Suppose `ptime` also printed a line to stdout before `fork()` (a debug line, say). Describe the specific wrong output a student would see if they used `exit()` in the child and had also skipped the fix from Q1. Pasting a real run is the best answer.

**(c)** In a program larger than this one, why is `return` from `main()` in the
child worse still?

_Your answer:_


---

## Q5. Whose CPU time did you measure? (4 points)

You called `getrusage(RUSAGE_CHILDREN, &ru)` once, after `waitpid` returned.

**(a)** Suppose `ptime` were changed to run the command three times in a row,
calling `getrusage(RUSAGE_CHILDREN, ...)` after each one. What would the third
call report — that run's CPU time, or something else? Say precisely what.

**(b)** Give a correct way to get *per-run* CPU time out of `RUSAGE_CHILDREN`
anyway.

**(c)** What would `getrusage(RUSAGE_SELF, ...)` have reported instead, and
roughly what number would you have seen in your report?

_Your answer:_


---

## Optional: stretch features

If you implemented any STRETCH items, list which ones and give one example
command line plus its output for each. Extra credit is not awarded for stretch
work that is not documented here.

_Your answer:_
