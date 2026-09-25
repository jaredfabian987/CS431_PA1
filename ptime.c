/* ===========================================================================
 * CS 433 Operating Systems -- Fall 2026 -- CSU San Marcos
 * PA1: ptime -- run a command as a child process and time it.
 *
 * NAME(S): <your name here>   (every member's name if you are working in a group)
 * DATE:    <date>
 * ===========================================================================
 *
 * This file compiles and runs AS GIVEN. Try it first:
 *
 *     make
 *     ./ptime /bin/echo hello
 *
 * It prints the report in the exact format the graders expect -- but every
 * number in it is a lie, because no child process has been created yet. Your
 * job is to make the numbers true. Search for "TODO" below; there are five.
 *
 * What is already written for you (do not rewrite these):
 *   signal_name()   signal number -> name, portable across Linux and macOS
 *   ts_to_sec()     struct timespec -> seconds as a double
 *   tv_to_sec()     struct timeval  -> seconds as a double
 *   print_report()  the exact output format. If you change it, tests fail.
 *
 * Build: gcc -Wall -Wextra -pthread -std=c11 -O2 -o ptime ptime.c
 * Your submission must compile with ZERO warnings under those exact flags.
 */

/* -std=c11 makes the compiler strictly standard, and glibc then hides every
 * POSIX function -- fork, execvp, clock_gettime, all of them. This line asks
 * for the POSIX.1-2008 interfaces back. It must come before any #include. */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>

/* ptime's own failure exit code: usage error, or a failed fork/waitpid/clock_gettime/getrusage.
 * Deliberately different from 126 and 127, which mean "the command you asked
 * for could not be run". */
#define PTIME_FAILURE 2

/* ------------------------------------------------------------------ *
 * Provided helpers
 * ------------------------------------------------------------------ */

/* We do not use strsignal() here: glibc returns "Terminated" for SIGTERM but
 * macOS returns "Terminated: 15", so the output would not be reproducible. */
static const char *signal_name(int sig)
{
    switch (sig) {
    case SIGHUP:  return "SIGHUP";
    case SIGINT:  return "SIGINT";
    case SIGQUIT: return "SIGQUIT";
    case SIGILL:  return "SIGILL";
    case SIGABRT: return "SIGABRT";
    case SIGFPE:  return "SIGFPE";
    case SIGKILL: return "SIGKILL";
    case SIGSEGV: return "SIGSEGV";
    case SIGPIPE: return "SIGPIPE";
    case SIGALRM: return "SIGALRM";
    case SIGTERM: return "SIGTERM";
    case SIGBUS:  return "SIGBUS";
    case SIGUSR1: return "SIGUSR1";
    case SIGUSR2: return "SIGUSR2";
    default:      return "unknown";
    }
}

static double ts_to_sec(const struct timespec *ts)
{
    return (double)ts->tv_sec + (double)ts->tv_nsec / 1e9;
}

static double tv_to_sec(const struct timeval *tv)
{
    return (double)tv->tv_sec + (double)tv->tv_usec / 1e6;
}

/* The exact required output. Seven lines, all on `out` (which must be
 * stderr). Field names are padded to 7 columns so the colons line up.
 * DO NOT CHANGE THIS FUNCTION -- tests/run_tests.sh compares against it. */
static void print_report(FILE *out, char *const cmd[], pid_t pid, int status,
                         double wall, double user, double sys)
{
    fprintf(out, "=== ptime ===\n");

    fprintf(out, "%-7s : ", "command");
    for (int i = 0; cmd[i] != NULL; i++)
        fprintf(out, "%s%s", (i == 0) ? "" : " ", cmd[i]);
    fputc('\n', out);

    fprintf(out, "%-7s : %ld\n", "pid", (long)pid);

    if (WIFEXITED(status)) {
        fprintf(out, "%-7s : exited %d\n", "status", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        fprintf(out, "%-7s : signaled %d (%s)\n", "status", sig,
                signal_name(sig));
    } else {
        fprintf(out, "%-7s : unknown 0x%x\n", "status", (unsigned)status);
    }

    fprintf(out, "%-7s : %.3f s\n", "wall", wall);
    fprintf(out, "%-7s : %.3f s\n", "user", user);
    fprintf(out, "%-7s : %.3f s\n", "sys",  sys);
}

/* ------------------------------------------------------------------ *
 * main -- this is the part you write
 * ------------------------------------------------------------------ */

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "ptime: usage: ptime COMMAND [ARG]...\n");
        return PTIME_FAILURE;
    }

    /* argv is always NULL-terminated by the C standard, so &argv[1] is
     * already a valid argument vector for execvp(). No copying needed. */
    char **cmd = &argv[1];

    /* ---------------------------------------------------------------
     * TODO 1 -- Read the clock BEFORE you create the child.
     *   clock_gettime(CLOCK_MONOTONIC, &t0);
     *   Check the return value. Why CLOCK_MONOTONIC and not CLOCK_REALTIME?
     *   Question 3 of analysis.md asks you.
     *
     * TODO 2 -- Empty the stdio buffers, then fork().
     *   One function call empties them. Run ./workload/buffer_trap first if
     *   you do not know which one or why (Question 1 of analysis.md).
     *   Handle fork() returning -1.
     *
     * TODO 3 -- In the child: execvp(cmd[0], cmd).
     *   execvp only returns if it FAILED. Save errno immediately, print
     *       ptime: cannot run 'NAME': STRERROR
     *   to stderr, and leave with _exit(127) if errno == ENOENT, otherwise
     *   _exit(126). Use _exit, not exit, and not return.
     *
     * TODO 4 -- In the parent: waitpid(pid, &status, 0).
     *   waitpid can fail with EINTR if a signal arrives while you wait; that
     *   is not a real error, so retry. Any other failure is fatal.
     *   Then read the clock again, and call
     *       getrusage(RUSAGE_CHILDREN, &ru)
     *   to get the child's user and system CPU time.
     *
     * TODO 5 -- Report and propagate.
     *   Call print_report(stderr, cmd, pid, status, wall, user, sys).
     *   Then return the child's exit code, or 128 + signal number if the
     *   child was killed by a signal. This is what your shell does, which is
     *   why `echo $?` after a Ctrl-C shows 130.
     * --------------------------------------------------------------- */

    fprintf(stderr, "ptime: STUB -- no child was created. "
                    "The report below is a placeholder.\n");

    /* Placeholder values so the stub compiles and runs. Once your fork/exec/
     * wait code is in place, t0 and t1 come from clock_gettime() and ru comes
     * from getrusage(); the three lines that call print_report stay as they
     * are. Converting the structs to seconds is boilerplate -- it is done for
     * you here so you can spend your time on the process lifecycle. */
    struct timespec t0 = {0, 0}, t1 = {0, 0};
    struct rusage ru;
    memset(&ru, 0, sizeof ru);
    int status = 0;                 /* the wait macros read this as "exited 0" */

    print_report(stderr, cmd, (pid_t)0, status,
                 ts_to_sec(&t1) - ts_to_sec(&t0),
                 tv_to_sec(&ru.ru_utime),
                 tv_to_sec(&ru.ru_stime));

    return PTIME_FAILURE;
}
