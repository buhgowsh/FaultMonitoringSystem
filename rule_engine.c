#include "rule_engine.h"

#include <ctype.h>
#include <math.h>
#include <string.h>

static bool compare(double actual, Compare cmp, double threshold)
{
    switch (cmp) {
    case CMP_LT: return actual < threshold;
    case CMP_LE: return actual <= threshold;
    case CMP_GT: return actual > threshold;
    case CMP_GE: return actual >= threshold;
    }
    return false;
}

RuleResult rule_matches_process(const Rule *rule, const ProcessInfo *process,
                                double ram_total_mib)
{
    if (!rule || !process || !rule->program_name || !rule->program_name[0] ||
        rule->n_conditions <= 0 || !rule->conditions ||
        rule->n_actions <= 0 || !rule->actions ||
        !memchr(process->commname, '\0', sizeof(process->commname)))
        return RULE_INVALID;

    if (strcmp(rule->program_name, process->commname) != 0)
        return RULE_NO_MATCH;

    for (int i = 0; i < rule->n_actions; ++i) {
        const Action *a = &rule->actions[i];
        if (a->kind < ACT_HALT || a->kind > ACT_DEC_PRIORITY ||
            ((a->kind == ACT_INC_PRIORITY || a->kind == ACT_DEC_PRIORITY) &&
             a->amount < 0))
            return RULE_INVALID;
    }

    bool unavailable = false, failed = false;
    for (int i = 0; i < rule->n_conditions; ++i) {
        const Condition *c = &rule->conditions[i];
        double actual;
        if (!isfinite(c->percent) || c->percent < 0 ||
            c->cmp < CMP_LT || c->cmp > CMP_GE)
            return RULE_INVALID;
        switch (c->resource) {
        case RES_CPU:
            if (!process->cpu_valid || !isfinite(process->cpu_percent) ||
                process->cpu_percent < 0) {
                unavailable = true;
                continue;
            }
            actual = process->cpu_percent;
            break;
        case RES_RAM:
            if (!isfinite(ram_total_mib) || ram_total_mib <= 0) {
                unavailable = true;
                continue;
            }
            actual = ((double)process->ram_bytes / (1024.0 * 1024.0))
                     / ram_total_mib * 100.0;
            if (!isfinite(actual)) {
                unavailable = true;
                continue;
            }
            break;
        case RES_VRAM:
            unavailable = true;
            continue; /* vsize_bytes is NOT GPU VRAM. */
        default:
            return RULE_INVALID;
        }
        if (!compare(actual, c->cmp, c->percent)) failed = true;
    }
    if (failed) return RULE_NO_MATCH;
    return unavailable ? RULE_UNAVAILABLE : RULE_MATCH;
}

size_t rule_engine_visit(const Rule *rules, int rule_count,
                      const ProcessSnapshot *snapshot, double ram_total_mib,
                      RuleActionVisitor visit, void *context)
{
    size_t matches = 0;
    if (!rules || rule_count <= 0 || !snapshot ||
        (snapshot->count && !snapshot->processes)) return 0;
    for (int r = 0; r < rule_count; ++r) {
        for (size_t p = 0; p < snapshot->count; ++p) {
            const ProcessInfo *process = &snapshot->processes[p];
            if (rule_matches_process(&rules[r], process, ram_total_mib) != RULE_MATCH)
                continue;
            ++matches;
            if (visit) {
                for (int a = 0; a < rules[r].n_actions; ++a)
                    visit((size_t)r, process, &rules[r].actions[a], context);
            }
        }
    }
    return matches;
}

void report_rule_action(size_t rule_index, const ProcessInfo *process,
                        const Action *action, void *context)
{
    FILE *out = context ? (FILE *)context : stdout;
    fprintf(out, "[REPORT ONLY] rule=%zu pid=%ld start=%llu name=\"",
            rule_index + 1, (long)process->pid,
            (unsigned long long)process->starttime_ticks);
    for (size_t i = 0; i < sizeof(process->commname) && process->commname[i]; ++i) {
        unsigned char c = (unsigned char)process->commname[i];
        if (isprint(c) && c != '"' && c != '\\') fputc(c, out);
        else fprintf(out, "\\x%02x", c);
    }
    fputs("\" action=", out);
    switch (action->kind) {
    case ACT_HALT: fputs("HALT (semantics pending)", out); break;
    case ACT_LOG: fputs("LOG", out); break;
    case ACT_INC_PRIORITY:
        fprintf(out, "INC_PRIORITY amount=%d", action->amount); break;
    case ACT_DEC_PRIORITY:
        fprintf(out, "DEC_PRIORITY amount=%d", action->amount); break;
    default: fputs("INVALID", out); break;
    }
    fputc('\n', out);
}

size_t rule_engine_evaluate(const ProcessSnapshot *snapshot)
{
    size_t matches = rule_engine_visit(g_rules, g_n_rules, snapshot,
                                       g_total_ram_mb, report_rule_action, stdout);
    fflush(stdout);
    return matches;
}


int rule_engine_init(int argc, char **argv)
{
    const char *path = NULL;
    double ram_mb = 0;
    double vram_mb = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ram-mb") == 0 && i + 1 < argc)
            ram_mb = atof(argv[++i]);
        else if (strcmp(argv[i], "--vram-mb") == 0 && i + 1 < argc)
            vram_mb = atof(argv[++i]);
        else
            path = argv[i];
    }

    // Initialize totals before parsing.
    if (ram_mb <= 0)
        ram_mb = detect_ram_mb();
    if (ram_mb <= 0)
        ram_mb = 8192;

    g_is_vram_assumed = false;

    if (vram_mb <= 0)
        vram_mb = detect_vram_mb();
    if (vram_mb <= 0) {
        vram_mb = 8192;
        g_is_vram_assumed = true;
    }

    g_total_ram_mb = ram_mb;
    g_total_vram_mb = vram_mb;

    int error = rules_parse(path);
    if (error != 0) {
        fprintf(stderr, "Rules parsing failed (code %d).\n", error);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
