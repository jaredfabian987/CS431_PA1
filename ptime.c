/* ===========================================================================
 * CS 433 Operating Systems -- Fall 2026 -- CSU San Marcos
 * PA1: ptime -- run a command as a child process and time it.
 *
 * NAME(S): <Jared Fabian, Cyril Tabaranza, Nathan Nguyen, Nick Pieratos, Kiernan Flieh>
 * DATE:    <09/28/2026>
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
    // no command was given, so there is nothing to run
    if (argc < 2) {
        fprintf(stderr, "ptime: usage: ptime COMMAND [ARG]...\n");
        return PTIME_FAILURE;
    }

    /* argv is always NULL-terminated by the C standard, so &argv[1] is
     * already a valid argument vector for execvp(). No copying needed. */
    char **cmd = &argv[1];


    // t0 is the time before the child starts, t1 is the time after it finishes
    struct timespec t0, t1;
    // read the starting time; CLOCK_MONOTONIC never jumps backward, so it is safe for timing
    if (clock_gettime(CLOCK_MONOTONIC, &t0) != 0) {
        // say what went wrong
        perror("ptime: clock_gettime");
        // stop the program, exit code 2
        return PTIME_FAILURE;
    }

    // print out anything we are holding onto before making a copy of ourselves
    // (fork copies our unflushed output buffer, so without this it prints twice
    // when stdout is a pipe or a file)
    // this comes after reading the clock so the flush is included in the timing
    fflush(NULL);

    // make a copy of this process, child gets 0, parent gets the child's ID
    pid_t childPid = fork();

    // if the fork failed
    if (childPid < 0){
        // say what went wrong
        perror("ptime: fork");
        // stop the program, exit code 2
        return PTIME_FAILURE;
    }

    // this part only runs in the child
    if (childPid == 0){
        // try to become the command we were asked to run
        execvp(cmd[0], cmd);

        // we only get here if execvp failed
        // a successful one never comes back, because it replaces this whole program
        // save the reason right away before anything else can erase it
        int execErrNo = errno;

        // say what happened
        fprintf(stderr, "ptime: cannot run '%s': %s\n", cmd[0], strerror(execErrNo));
        // command doesn't exist
        if (execErrNo == ENOENT){
            // exit code 127 if the command was not found
            // _exit (not exit) so we don't flush the buffers we inherited from the parent
            _exit(127);
        } else {
            // command exists but couldn't be run
            // exit code 126 for any other error
            _exit(126);
        }
    } // end of childPid == 0

    // will hold how the child ended
    int status;

    // wait for that specific child to finish
    pid_t res = waitpid(childPid, &status, 0);

    // if we got interrupted by a signal, that is not a real error
    while (res < 0 && errno == EINTR){
        // just try waiting again
        res = waitpid(childPid, &status, 0);
    } // end of while EINTR

    // any other failure is real
    if (res < 0) {
        // say what went wrong and stop the program, exit code 2
        perror("ptime: waitpid");
        return PTIME_FAILURE;
    }

    // stop the stopwatch now that the child is done
    // this must come after waitpid returns, or the wall time would be too small
    if (clock_gettime(CLOCK_MONOTONIC, &t1) != 0) {
        // say what went wrong and stop the program, exit code 2
        perror("ptime: clock_gettime");
        return PTIME_FAILURE;
    }

    // will hold the CPU usage info from the kernel
    struct rusage r;
    // ask how much CPU time our children used (RUSAGE_SELF would measure ptime itself)
    if (getrusage(RUSAGE_CHILDREN, &r) != 0) {
        // say what went wrong and stop the program, exit code 2
        perror("ptime: getrusage");
        return PTIME_FAILURE;
    }

    // total real time the child took
    double wall = ts_to_sec(&t1) - ts_to_sec(&t0);
    // CPU time spent running the child's own code
    double user = tv_to_sec(&r.ru_utime);
    // CPU time the kernel spent working for the child
    double sys = tv_to_sec(&r.ru_stime);

    // print the final report (to stderr, so the child's output stays clean)
    print_report(stderr, cmd, childPid, status, wall, user, sys);

    // did the child finish on its own
    if (WIFEXITED(status)) {
        // pass its exit code straight through
        return WEXITSTATUS(status);
    } // end of WIFEXITED

    // was the child killed by a signal instead
    if (WIFSIGNALED(status)) {
        // shell convention is 128 + signal number
        return 128 + WTERMSIG(status);
    } // end of WIFSIGNALED

    return PTIME_FAILURE;  // should never happen, but just in case
}
