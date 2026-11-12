#define _POSIX_C_SOURCE 200809L
#include "readproc.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "rules.h"
#include "rule_engine.h"
#include <unistd.h>

static volatile sig_atomic_t running = 1;

static void request_shutdown(int signal_number)
{
    (void)signal_number;
    running = 0;
}

static void sleep_one_second(void)
{
    struct timespec remaining = {.tv_sec = 1, .tv_nsec = 0};
    while (running && nanosleep(&remaining, &remaining) == -1) {
        if (errno != EINTR) {
            perror("nanosleep");
            running = 0;
            break;
        }
    }
}

static void display_snapshot(const ProcessSnapshot *snapshot)
{
    printf("\nProcesses sampled: %zu\n", snapshot->count);
    printf("%-8s %-20s %9s %10s %12s\n",
           "PID", "NAME", "CPU %", "RAM MB", "VmSize MB");

    // Display every readable process, in PID order (set by readproc_collect).
    for (size_t i = 0; i < snapshot->count; ++i) {
        const ProcessInfo *p = &snapshot->processes[i];

        printf("%-8ld %-20.20s ", (long)p->pid, p->commname);
        if (p->cpu_valid)
            printf("%9.1f ", p->cpu_percent);
        else
            printf("%9s ", "N/A");

        // Decimal megabytes (MB): 1 MB = 1,000,000 bytes.
        printf("%10.2f %12.2f\n",
               (double)p->ram_bytes / 1000000.0,
               (double)p->vsize_bytes / 1000000.0);
    }
    fflush(stdout);
}

int main(int argc, char **argv)
{
    
    if (rule_engine_init(argc, argv) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    if (readproc_init() != 0) {
        fprintf(stderr, "Unable to read Linux clock ticks or page size.\n");
        return EXIT_FAILURE;
    }

    // Ctrl+C or SIGTERM stops the sampling loop cleanly.
    struct sigaction action = {.sa_handler = request_shutdown};
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) == -1 ||
        sigaction(SIGTERM, &action, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    ProcessSnapshot previous = {0};
    if (readproc_collect(&previous) != 0) {
        perror("readproc_collect initial");
        return EXIT_FAILURE;
    }

    printf("Monitoring /proc every ~1 second; _SC_CLK_TCK = %ld\n",
           readproc_clock_ticks());
    printf("CPU%% uses 100%% per fully used logical core. Press Ctrl+C to stop.\n");

    int result = EXIT_SUCCESS;
    while (running) {
        sleep_one_second();
        if (!running) break;

        ProcessSnapshot current = {0};
        if (readproc_collect(&current) != 0) {
            perror("readproc_collect");
            result = EXIT_FAILURE;
            break;
        }

        readproc_calculate_cpu(&current, &previous);
        display_snapshot(&current);
        // Later: evaluate rules and log any actual violations here.

        rule_engine_evaluate(&current);

        readproc_free_snapshot(&previous);
        previous = current; // Takes ownership of the new allocation.
    }

    readproc_free_snapshot(&previous);
    puts("\nMonitoring stopped.");
    return result;
}
