#define _POSIX_C_SOURCE 200809L
#include "readproc.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STAT_BUF_SIZE 8192

static long ticks_per_second = 0;
static long page_size = 0;

int readproc_init(void)
{
    ticks_per_second = sysconf(_SC_CLK_TCK);
    page_size = sysconf(_SC_PAGESIZE);
    return (ticks_per_second > 0 && page_size > 0) ? 0 : -1;
}

long readproc_clock_ticks(void)
{
    return ticks_per_second;
}

static int parse_ulong_long(const char *text, uint64_t *value)
{
    if (!text || *text == '-' || *text == '\0') return -1;
    errno = 0;
    char *end;
    unsigned long long parsed = strtoull(text, &end, 10);
    if (errno != 0 || *end != '\0') return -1;
    *value = (uint64_t)parsed;
    return 0;
}

static int parse_long(const char *text, long *value)
{
    if (!text || *text == '\0') return -1;
    errno = 0;
    char *end;
    long parsed = strtol(text, &end, 10);
    if (errno != 0 || *end != '\0') return -1;
    *value = parsed;
    return 0;
}

static int parse_stat(pid_t pid, char *line, ProcessInfo *info)
{
    // Field 2 (comm) is in parentheses and may contain spaces.
    char *left = strchr(line, '(');
    char *right = strrchr(line, ')');
    if (!left || !right || right <= left) return -1;

    memset(info, 0, sizeof(*info));
    info->pid = pid;

    size_t name_length = (size_t)(right - left - 1);
    if (name_length >= sizeof(info->commname))
        name_length = sizeof(info->commname) - 1;
    memcpy(info->commname, left + 1, name_length);
    info->commname[name_length] = '\0';

    // Fields 3 to 24. Index 0 is field 3 (state).
    char *fields[22];
    size_t field_count = 0;
    char *saveptr = NULL;
    for (char *token = strtok_r(right + 1, " \t\n", &saveptr);
         token && field_count < 22;
         token = strtok_r(NULL, " \t\n", &saveptr)) {
        fields[field_count++] = token;
    }
    if (field_count != 22) return -1;

    long parent_pid;
    if (parse_long(fields[1], &parent_pid) != 0 ||
        parent_pid < 0 || parent_pid > INT_MAX ||
        parse_long(fields[15], &info->priority) != 0 ||
        parse_long(fields[16], &info->nice) != 0 ||
        parse_long(fields[21], &info->rss_pages) != 0 ||
        parse_ulong_long(fields[11], &info->utime_ticks) != 0 ||
        parse_ulong_long(fields[12], &info->stime_ticks) != 0 ||
        parse_ulong_long(fields[19], &info->starttime_ticks) != 0 ||
        parse_ulong_long(fields[20], &info->vsize_bytes) != 0)
        return -1;

    info->ppid = (pid_t)parent_pid;
    info->state = fields[0][0];
    if (info->rss_pages > 0) {
        uint64_t pages = (uint64_t)info->rss_pages;
        if (pages > UINT64_MAX / (uint64_t)page_size) return -1;
        info->ram_bytes = pages * (uint64_t)page_size;
    }
    return 0;
}

static int read_one_process(pid_t pid, ProcessInfo *info)
{
    char filename[64];
    int length = snprintf(filename, sizeof(filename), "/proc/%ld/stat", (long)pid);
    if (length < 0 || (size_t)length >= sizeof(filename)) return -1;

    FILE *file = fopen(filename, "r");
    if (!file) return -1; // Process may have exited or be unreadable.

    char line[STAT_BUF_SIZE];
    char *result = fgets(line, sizeof(line), file);
    fclose(file);
    if (!result || !strchr(line, '\n')) return -1;
    return parse_stat(pid, line, info);
}

static int compare_pid(const void *a, const void *b)
{
    const ProcessInfo *x = a;
    const ProcessInfo *y = b;
    return (x->pid > y->pid) - (x->pid < y->pid);
}

int readproc_collect(ProcessSnapshot *out)
{
    if (!out || ticks_per_second <= 0 || page_size <= 0) return -1;

    DIR *dir = opendir("/proc");
    if (!dir) return -1;

    ProcessSnapshot snapshot = {0};
    size_t capacity = 0;

    for (;;) {
        errno = 0;
        struct dirent *entry = readdir(dir);
        if (!entry) {
            if (errno != 0) goto fail;
            break;
        }

        // Process directories are numeric: /proc/1234.
        const char *name = entry->d_name;
        bool numeric = (*name != '\0');
        for (const char *p = name; *p != '\0'; ++p) {
            if (!isdigit((unsigned char)*p)) {
                numeric = false;
                break;
            }
        }
        if (!numeric) continue;

        long pid_value;
        if (parse_long(name, &pid_value) != 0 ||
            pid_value <= 0 || pid_value > INT_MAX)
            continue;

        ProcessInfo info;
        if (read_one_process((pid_t)pid_value, &info) != 0)
            continue;

        if (snapshot.count == capacity) {
            size_t new_capacity = capacity ? capacity * 2 : 128;
            if (new_capacity < capacity ||
                new_capacity > SIZE_MAX / sizeof(*snapshot.processes))
                goto fail;
            ProcessInfo *expanded = realloc(snapshot.processes,
                                             new_capacity * sizeof(*expanded));
            if (!expanded) goto fail;
            snapshot.processes = expanded;
            capacity = new_capacity;
        }
        snapshot.processes[snapshot.count++] = info;
    }

    closedir(dir);
    if (clock_gettime(CLOCK_MONOTONIC, &snapshot.sampled_at) != 0)
        goto fail_no_dir;
    qsort(snapshot.processes, snapshot.count, sizeof(*snapshot.processes), compare_pid);
    *out = snapshot;
    return 0;

fail:
    closedir(dir);
fail_no_dir:
    free(snapshot.processes);
    return -1;
}

void readproc_calculate_cpu(ProcessSnapshot *current,
                            const ProcessSnapshot *previous)
{
    if (!current || !previous || !previous->processes || ticks_per_second <= 0)
        return;

    double elapsed =
        (double)(current->sampled_at.tv_sec - previous->sampled_at.tv_sec) +
        (double)(current->sampled_at.tv_nsec - previous->sampled_at.tv_nsec) / 1e9;
    if (elapsed <= 0.0) return;

    for (size_t i = 0; i < current->count; ++i) {
        ProcessInfo *now = &current->processes[i];
        ProcessInfo key = {.pid = now->pid};
        const ProcessInfo *before = bsearch(&key, previous->processes,
                                            previous->count, sizeof(*before),
                                            compare_pid);
        // starttime protects against a PID being reused by a new process.
        if (!before || before->starttime_ticks != now->starttime_ticks)
            continue;

        uint64_t now_ticks = now->utime_ticks + now->stime_ticks;
        uint64_t before_ticks = before->utime_ticks + before->stime_ticks;
        if (now_ticks < before_ticks) continue;

        now->cpu_percent = 100.0 * (double)(now_ticks - before_ticks)
                           / ((double)ticks_per_second * elapsed);
        now->cpu_valid = true;
    }
}

void readproc_free_snapshot(ProcessSnapshot *snapshot)
{
    if (!snapshot) return;
    free(snapshot->processes);
    *snapshot = (ProcessSnapshot){0};
}
