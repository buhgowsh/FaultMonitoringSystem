#ifndef RULES_H
#define RULES_H

#include <glob.h>    // VRAM detection
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // RAM detection

typedef enum { RES_CPU, RES_RAM, RES_VRAM } Resource;

// A condition with no operator (such as "ram 512mb") is treated as >= or CMP_GT
typedef enum { CMP_LT, CMP_LE, CMP_GT, CMP_GE } Compare;

typedef enum { ACT_HALT, ACT_LOG, ACT_INC_PRIORITY, ACT_DEC_PRIORITY } ActionKind;

// TODO discuss if percent as better as a value between 0.0 and 1.0
// "ram > 512mb" is stored as { RES_RAM, CMP_GT, <512 / total_ram_mb * 100> }
typedef struct {
    Resource resource;
    Compare  cmp;
    double   percent;
} Condition;

typedef struct {
    ActionKind kind;
    int        amount; // # after inc/dec; default is 1 (unused for halt/log)
} Action;

// For each rule: when ALL conditions hold, do ALL actions to program; TODO get rid of mult actions
typedef struct {
    char      *program_name; // NULL if no header preceded it; TODO discuss how to handle
    Condition *conditions;
    int        n_conditions;
    Action    *actions;
    int        n_actions;
} Rule;

// Result of a successful parse
extern Rule *g_rules;
extern int   g_n_rules;

int rules_parse(const char *path);

// Totals used to turn mb/gb into percentages
//   set before yr_parse() is called
extern double g_total_ram_mb;
extern double g_total_vram_mb;
extern bool g_is_vram_assumed;

double detect_ram_mb(void);
double detect_vram_mb(void);

#endif /* RULES_H */
