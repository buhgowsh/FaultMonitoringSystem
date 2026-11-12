#ifndef READPROC_H
#define READPROC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

typedef struct {
    pid_t pid;
    pid_t ppid;
    char commname[64];
    char state;
    long priority;
    long nice;

    uint64_t utime_ticks;
    uint64_t stime_ticks;
    uint64_t starttime_ticks;
    uint64_t vsize_bytes;
    long rss_pages;
    uint64_t ram_bytes;

    double cpu_percent;
    bool cpu_valid;  // false for a new process without a previous sample
} ProcessInfo;

typedef struct {
    ProcessInfo *processes;
    size_t count;
    struct timespec sampled_at;
} ProcessSnapshot;

// Call once at startup. Returns 0 on success, -1 on error.
int readproc_init(void);
long readproc_clock_ticks(void);

// Collects a new snapshot. 'out' must be initialized to {0}.
// Caller must release its memory with readproc_free_snapshot().
int readproc_collect(ProcessSnapshot *out);

// Sets CPU percentages in 'current' using previous snapshot.
void readproc_calculate_cpu(ProcessSnapshot *current,
                            const ProcessSnapshot *previous);

void readproc_free_snapshot(ProcessSnapshot *snapshot);

#endif
