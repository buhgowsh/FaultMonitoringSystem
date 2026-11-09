#include "rules.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

int main(int argc, char **argv) {
    const char *path = NULL;
    double ram_mb = 0, vram_mb = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ram-mb") == 0 && i + 1 < argc)
            ram_mb = atof(argv[++i]);
        else if (strcmp(argv[i], "--vram-mb") == 0 && i + 1 < argc)
            vram_mb = atof(argv[++i]);
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
		printf("PARSE SUCCEEDED\n");
		print_rules();
		break;
	case 1:
		printf("PARSE FAILED\n");
		return 1;
	case 2:
		printf("ERROR: File %s doesn't exist\n", path);
		return 2;
	}
    return 0;
}
