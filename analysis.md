# PA1 Written Analysis

**Name(s):** _(Kiernan Flieh, Cyril Tabaranza, Nathan Nguyen, Jared Fabian, Nick Pieratos)_
**Date: *9/28/2026*
**Built and tested on:** _(built on Ubuntu (WSL) GCC version 13.3.0 and tested on the CSUSM CS433 course server. GCC version 11.5.0)_

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

./workload/buffer_trap OUTPUT: 
A: printed before fork()
B: child
C: parent
./workload/buffer_trap | cat OUTPUT: 
A: printed before fork()
B: child
A: printed before fork()
C: parent

**(b)** They are different, and the program did not change. Explain why, in
terms of where `printf` actually writes and what `fork()` copies.

I believe its because printf() first writes output into a stdio buffer stored in the process's memory. When stdout is connected to a terminal, it is usually line-buffered, so the newline causes the first line to be flushed before fork(). When stdout is piped to cat, the output can remain in the buffer, so fork() copies that buffer into both the parent and child, causing both processes to eventually print the same buffered line.

**(c)** Name the one library call that fixes it, say exactly where it goes in
`ptime.c`, and explain why placing it *after* the `fork()` would not work.

The library call that fixes this is fflush(NULL). It should be placed immediately before fork() in ptime.c so all stdio buffers are flushed before the child is created. If it were placed after fork(), the child would already have its own copy of the buffered output, so both processes could still flush and print it.

---

## Q2. Wall time is not CPU time (4 points)

Run both of these with your finished `ptime` and paste both reports:

```
./ptime ./workload/sleeper 1000
./ptime ./workload/spin 1000
```

**(a)** For each one, say which is larger — wall, or user+sys — and why.
./ptime ./workload/sleeper 1000 OUTPUT:
sleeper: slept 1000 ms
=== ptime ===
command : ./workload/sleeper 1000
pid     : 2001703
status  : exited 0
wall    : 1.003 s
user    : 0.001 s
sys     : 0.000 s

For sleeper, wall time is much larger than user + sys time because the program spends most of its time sleeping instead of actively using the CPU. 

./ptime ./workload/spin 1000 OUTPUT: 
spin: burned 1000 ms of CPU (checksum 391680000)
=== ptime ===
command : ./workload/spin 1000
pid     : 2002024
status  : exited 0
wall    : 1.003 s
user    : 1.074 s
sys     : 0.000 s


For spin, user + sys is close to the wall time because the program is CPU-bound and spends most of the run actively executing on the CPU.

**(b)** A program can finish with `user + sys` *greater* than `wall`. Describe
a program that would do that. (None of the provided workload programs on its own will do it; say what
kind of program would, and what hardware makes it possible. You can check your answer with
`./ptime sh -c "./workload/spin 500 & ./workload/spin 500 & wait"`.)

A program can have user + sys be greater than wall time if it runs multiple processes or threads at the same time on different CPU cores. CPU time from each core is added together, even though those processes are running during the same wall-clock interval. For example, two CPU-bound processes running simultaneously on two cores could use about one second of CPU time each while only about one second of wall time passes.

---

## Q3. Which clock, and why (4 points)

You used `CLOCK_MONOTONIC`. `CLOCK_REALTIME` also exists, and it reports the
wall-clock time of day.

**(a)** Describe a concrete situation in which measuring an interval with
`CLOCK_REALTIME` would give a wrong answer — including one where the measured
duration comes out *negative*.

CLOCK_REALTIME would give you the wrong answer if the system clock changed while the program runs. This would cause you to get a negative time if the clock is 12:00 and is changed to 11:59 causing the time calculated to be -60s. 

**(b)** Given that, why does `CLOCK_REALTIME` exist at all? Name one job it is
right for and `CLOCK_MONOTONIC` is wrong for.

CLOCK_REALTIME is really good for logging since it shows the current date and time. CLOCK_MONOTONIC would be bad at this task since it only measures time passed. 

---

## Q4. `_exit` versus `exit` in the child (4 points)

Your child process calls `_exit()` after a failed `execvp()` — never `exit()`,
and never `return`.

**(a)** What does `exit()` do that `_exit()` does not?

exit() performs normal process cleanup before terminating, including flushing stdio buffers and running functions registered with atexit(). _exit() terminates the process immediately without flushing those inherited stdio buffers or running normal user-space cleanup.

**(b)** Connect this to Q1. Suppose `ptime` also printed a line to stdout before `fork()` (a debug line, say). Describe the specific wrong output a student would see if they used `exit()` in the child and had also skipped the fix from Q1. Pasting a real run is the best answer.

If ptime printed a debug line before fork() without flushing it first, both the parent and child would receive a copy of that buffered line. If execvp() failed and the child used exit(), the child's copy would be flushed, and the parent would later flush its own copy, causing the debug line to appear twice.

kiernan@K:/mnt/c/Users/Kiern/Downloads/PA1_starter/PA1_starter$ ./ptime this_command_does_not_exist
ptime: cannot run 'this_command_does_not_exist': No such file or directory
Debug: before fork=== ptime ===
command : this_command_does_not_exist
pid     : 2006446
status  : exited 127
wall    : 0.017 s
user    : 0.000 s
sys     : 0.002 s
Debug: before forkkiernan@K:/mnt/c/Users/Kiern/Downloads/PA1_starter/PA1_starter$

The debug line was only printed once in the program, but it appeared twice because both processes had a copy of the buffered output.


**(c)** In a program larger than this one, why is `return` from `main()` in the
child worse still?

Returning from main() performs normal program termination similar to calling exit(), so it can flush stdio buffers and run cleanup handlers. In a larger program, those cleanup handlers could do more than print output, such as writing files or changing other resources, causing the child to repeat cleanup that should only be done by the parent. _exit() avoids those unwanted actions by terminating the child immediately. 

---

## Q5. Whose CPU time did you measure? (4 points)

You called `getrusage(RUSAGE_CHILDREN, &ru)` once, after `waitpid` returned.

**(a)** Suppose `ptime` were changed to run the command three times in a row,
calling `getrusage(RUSAGE_CHILDREN, ...)` after each one. What would the third
call report — that run's CPU time, or something else? Say precisely what.

The third call would report the accumulated CPU time of all terminated and waited-for children so far, not just the third child. RUSAGE_CHILDREN keeps a cumulative total instead of resetting after each child.

**(b)** Give a correct way to get *per-run* CPU time out of `RUSAGE_CHILDREN`
anyway.

A correct way to get per-run CPU time is to call getrusage(RUSAGE_CHILDREN, ...) before starting a run and save those values. After the child finishes, call it again and subtract the earlier user and system times from the new values. The difference gives the CPU usage for that individual run.

**(c)** What would `getrusage(RUSAGE_SELF, ...)` have reported instead, and
roughly what number would you have seen in your report?

So, getrusage(RUSAGE_SELF, ...) would report the CPU time used by the ptime process itself instead of the child command. The value would normally be very small, around 0.000 seconds or a few thousandths of a second, because ptime spends most of its time waiting for the child instead of doing CPU-intensive work.
---

## Optional: stretch features

If you implemented any STRETCH items, list which ones and give one example
command line plus its output for each. Extra credit is not awarded for stretch
work that is not documented here.

_Your answer:_

