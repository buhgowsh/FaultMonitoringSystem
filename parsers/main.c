#include "config.h"
#include "rules.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *res_name(Resource r) {
    return r == RES_CPU ? "cpu" : r == RES_RAM ? "ram" : "vram";
}

static const char *cmp_name(Compare c) {
    switch (c) {
    case CMP_LT:
        return "<";
    case CMP_LE:
        return "<=";
    case CMP_GT:
        return ">";
    default: // CMP_GE
        return ">=";
    }
}

// Debug testing
static void print_rules(void) {
    for (int i = 0; i < g_n_rules; i++) {
        const Rule *r = &g_rules[i];
        printf("rule %d  program: %s\n", i + 1, r->program_name ? r->program_name : "(none)");
        printf("  when:");
        for (int j = 0; j < r->n_conditions; j++)
            printf("%s %s %s %.2f%%", j ? " and" : "",
                   res_name(r->conditions[j].resource),
                   cmp_name(r->conditions[j].cmp),
                   r->conditions[j].percent);
        printf("\n  then:");
        for (int j = 0; j < r->n_actions; j++) {
            const Action *a = &r->actions[j];
            printf("%s ", j ? ";" : "");
            switch (a->kind) {
            case ACT_HALT:         printf("halt"); break;
            case ACT_LOG:          printf("log"); break;
            case ACT_INC_PRIORITY: printf("increment priority by %d", a->amount); break;
            case ACT_DEC_PRIORITY: printf("decrement priority by %d", a->amount); break;
            }
        }
        printf("\n");
    }
}

static const char *sound_name(SoundOpts s) {
    return s == S_NONE ? "none" : "bell";
}

static const char *logopen_name(LogOpenOpts l) {
    return l == LO_NEVER ? "never" : "always";
}

static const char *logclose_name(LogCloseOpts l) {
    switch (l) {
    case LC_NEVER:
        return "never";
    case LC_CRASH:
        return "crashes";
    default: /* LC_ALWAYS */
        return "always";
    }
}

static const char *logsplit_name(LogSplitOpts l) {
    switch (l) {
    case LS_NEVER:
        return "never";
    case LS_DAILY:
        return "daily";
    case LS_WEEKLY:
        return "weekly";
    default: /* LS_MONTHLY */
        return "monthly";
    }
}

// Debug testing
static void print_config(void) {
    printf("log_split: %s\n",  logsplit_name(g_config.logsplit));
    printf("max_logs: %u\n",   g_config.maxlog);
    printf("log_open: %s\n",   logopen_name(g_config.logopen));
    printf("log_close: %s\n",  logclose_name(g_config.logclose));
    printf("halt_warning: %u\n",  g_config.haltwarn);
    printf("log_sound: %s\n",  sound_name(g_config.logsound));
    printf("halt_sound: %s\n", sound_name(g_config.haltsound));
    printf("inc_sound: %s\n",  sound_name(g_config.incsound));
    printf("dec_sound: %s\n",  sound_name(g_config.decsound));
}

int main(int argc, char **argv) {
    const char *path = NULL;
    double ram_mb = 0, vram_mb = 0;
	const char *config_path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ram-mb") == 0 && i + 1 < argc)
            ram_mb = atof(argv[++i]);
        else if (strcmp(argv[i], "--vram-mb") == 0 && i + 1 < argc)
            vram_mb = atof(argv[++i]);
		else if (strcmp(argv[i], "--config-file") == 0 && i + 1 < argc)
			config_path = argv[++i];
        else
            path = argv[i];
    }

    if (ram_mb <= 0)  ram_mb = detect_ram_mb();
    if (ram_mb <= 0)  ram_mb = 8192;
    if (vram_mb <= 0) vram_mb = detect_vram_mb();
    if (vram_mb <= 0) { vram_mb = 8192; g_is_vram_assumed = true; }
    g_total_ram_mb = ram_mb;
    g_total_vram_mb = vram_mb;

	int error = rules_parse(path);
	switch (error) {
	case 0:
		printf("RULES PARSE SUCCEEDED\n");
		print_rules();
		break;
	case 1:
		printf("RULES PARSE FAILED\n");
		return 1;
	case 2:
		printf("ERROR: File %s doesn't exist\n", path);
		return 2;
	}

	// CONFIG
	error = config_parse(config_path);
	switch (error) {
	case 0:
		printf("CONFIG PARSE SUCCEEDED\n");
		print_config();
		break;
	case 1:
		printf("CONFIG PARSE FAILED\n");
		return 1;
	case 2:
		printf("ERROR: File %s doesn't exist\n", path);
		return 2;
	}
    return 0;
}
