/* CS 433 PA1 -- provided workload. You do not need to modify this file.
 *
 * spin MS -- burn about MS milliseconds of CPU time in user mode. It is the
 * opposite of sleeper: it never sleeps, so wall time and user CPU time should
 * both be about MS.
 *
 *   ./ptime ./workload/spin 500  -> wall ~= 0.5 s, user ~= 0.5 s
 *
 * Build: gcc -Wall -Wextra -pthread -std=c11 -O2 -o spin spin.c
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
 
int main(int argc, char *argv[])
{
    // default if no time is given
    long ms = 300;
    if (argc > 1)
        ms = strtol(argv[1], NULL, 10);
    // a negative time makes no sense
    if (ms < 0)
        ms = 0;
 
    // how long to spin, in seconds
    double target = (double)ms / 1000.0;
 
    // t0 is when we started, now is the latest time check
    struct timespec t0, now;
    clock_gettime(CLOCK_MONOTONIC, &t0);
 
    // volatile stops the compiler from deleting the loop, so it really uses the CPU
    volatile double acc = 0.0;
    double elapsed = 0.0;
 
    // keep working until enough real time has passed
    while (elapsed < target) {
        // do a chunk of pointless math (this is the CPU time being burned)
        for (int i = 0; i < 20000; i++)
            acc = acc + 1.0;
        // check the clock only once per chunk, since checking is slow
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed = (double)(now.tv_sec - t0.tv_sec)
                + (double)(now.tv_nsec - t0.tv_nsec) / 1e9;
    } // end of while
 
    // print the result so the compiler can't say the work was unused
    printf("spin: burned %ld ms of CPU (checksum %.0f)\n", ms, (double)acc);
    return 0;
}