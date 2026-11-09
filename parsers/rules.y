%code requires {
#include "rules.h"
}

%{

#include "rules.h"

// Must define these else error is thrown by the compiler
int yr_lex(void);
void yr_error(const char *msg);
extern int yr_lineno;
extern FILE *yr_in;

Rule  *g_rules = NULL;
int    g_n_rules = 0;
double g_total_ram_mb = 0;
double g_total_vram_mb = 0;
bool  g_is_vram_assumed = false;

// Current header for each new rule
static char *g_title = NULL;

// Out of memory; hopefully shouldn't happen
static void oom(void) {
    perror("memory");
    exit(3);
}

static Rule *rule_new(void) {
    Rule *r = calloc(1, sizeof(*r));
    if (!r) { oom(); }
    return r;
}

static void rule_add_condition(Rule *r, Condition c) {
    r->conditions = realloc(r->conditions, (r->n_conditions + 1) * sizeof(c));
    if (!r->conditions) { oom(); }
    r->conditions[r->n_conditions++] = c;
}

static void rule_add_action(Rule *r, Action a) {
    r->actions = realloc(r->actions, (r->n_actions + 1) * sizeof(a));
    if (!r->actions) { oom(); }
    r->actions[r->n_actions++] = a;
}

// Move a finished rule into the global list
static void rule_commit(Rule *r) {
    g_rules = realloc(g_rules, (g_n_rules + 1) * sizeof(*g_rules));
    if (!g_rules) { oom(); }
    // Adds the current title
    r->program_name = g_title
        ? strdup(g_title)
        : NULL;
    g_rules[g_n_rules++] = *r;
    free(r);
}

// Converts the mb of a resource -> percent of its total.
static double mb_to_percent(Resource res, double mb) {
    double total = (res == RES_VRAM)
        ? g_total_vram_mb
        : g_total_ram_mb;
    if (res == RES_VRAM && g_is_vram_assumed) {
        fprintf(stderr, "note: VRAM size not detected, assuming %.0f MB "
                        "(override with --vram-mb)\n", total);
        g_is_vram_assumed = false;
    }
    return mb / total * 100.0;
}
%}

%define parse.error verbose

%union {
    long        num;
    double      dbl;
    char       *str;
    Resource    res;
    Compare     cmp;
    ActionKind  kind;
    Condition   cond;
    Action      act;
    Rule       *rule;
}

%token <str> WORD
%token <num> NUMBER
%token <cmp> OP
%token <dbl> SIZE
%token <res> RAM
%token PERCENT IF THEN AND HALT LOG CPU PRIORITY INC DEC

%type <cmp>  opt_op
%type <dbl>  percentage num_bytes
%type <num>  opt_number
%type <kind> step
%type <cond> condition
%type <act>  action
%type <rule> conditions if_then then_if

%start rules

%%

rules
    : %empty
    | rules item
    ;

item
    : WORD          { free(g_title); g_title = $1; }       /* <header> */
    | if_then       { rule_commit($1); }
    | then_if       { rule_commit($1); }
    ;

if_then
    : IF conditions THEN action     { rule_add_action($2, $4); $$ = $2; }
    | if_then THEN action           { rule_add_action($1, $3); $$ = $1; }
    ;

then_if
    : action IF conditions          { rule_add_action($3, $1); $$ = $3; }
    ;

conditions
    : condition                     { $$ = rule_new(); rule_add_condition($$, $1); }
    | conditions AND condition      { rule_add_condition($1, $3); $$ = $1; }
    ;

condition
    : CPU opt_op percentage         { $$.resource = RES_CPU; $$.cmp = $2; $$.percent = $3; }
    | RAM opt_op percentage         { $$.resource = $1;      $$.cmp = $2; $$.percent = $3; }
    | RAM opt_op num_bytes          { $$.resource = $1;      $$.cmp = $2;
                                      $$.percent = mb_to_percent($1, $3); }
    ;

opt_op
    : %empty        { $$ = CMP_GE; }
    | OP            { $$ = $1; }
    ;

percentage : NUMBER PERCENT         { $$ = (double)$1; } ;
num_bytes  : NUMBER SIZE            { $$ = (double)$1 * $2; } ;   /* in MB */

action
    : HALT                          { $$.kind = ACT_HALT; $$.amount = 0; }
    | LOG                           { $$.kind = ACT_LOG;  $$.amount = 0; }
    | step opt_priority opt_number  { $$.kind = $1; $$.amount = ($3 < 0) ? 1 : (int)$3; }
    ;

step
    : INC           { $$ = ACT_INC_PRIORITY; }
    | DEC           { $$ = ACT_DEC_PRIORITY; }
    ;

opt_priority : %empty | PRIORITY ;

opt_number
    : %empty        { $$ = -1; }
    | NUMBER        { $$ = $1; }
    ;

%%

void yr_error(const char *msg) {
    fprintf(stderr, "line %d: %s\n", yr_lineno, msg);
}

// TODO Rename these functions to show what file they are in and
//   move them to a more appropriate file
/* ---------- system sizes ---------- */

double detect_ram_mb(void) {
    long pages = sysconf(_SC_PHYS_PAGES), size = sysconf(_SC_PAGE_SIZE);
    if (pages <= 0 || size <= 0) return 0;
    return (double)pages * (double)size / (1024.0 * 1024.0);
}

// TODO check with nvidia gpus
double detect_vram_mb(void) {
    glob_t g;
    double best = 0;
    // This is where amd gpus store vram
    if (glob("/sys/class/drm/card*/device/mem_info_vram_total", 0, NULL, &g) == 0) {
        for (size_t i = 0; i < g.gl_pathc; i++) {
            FILE *f = fopen(g.gl_pathv[i], "r");
            unsigned long long bytes;
            if (f && fscanf(f, "%llu", &bytes) == 1) {
                double mb = (double)bytes / (1024.0 * 1024.0);
                if (mb > best) {
                    best = mb;
                }
            }
            if (f) {
                fclose(f);
            }
        }
    }
    globfree(&g);
    return best;
}

//

// Parse the given file
//   Parses the default rule file if path is null
//   Returns: 0 when things go smooth
//            1 when the parsing fails
//            2 if the file wouldn't open
int rules_parse(const char *path) {
    if (!path) {
        // Get path to home dir
        char* home = getenv("HOME");
        if (!home) { oom(); }
        // TODO discuss name and file extension for rule file
        path = "/.config/fms/rules";

        // TODO could make more efficient
        // Make a string big enough to support the concat of home and path
        int len = strlen(home) + strlen(path) + 1;
        char* default_path = malloc(len * sizeof(char));
        if (!default_path) { oom(); }
        default_path[0] = '\0';     // Makes sure strcat works correctly
        default_path[len-1] = '\0'; // strcat doesn't add null byte (I believe)

        default_path = strcat(default_path, home);
        path = strcat(default_path, path);
    }
    yr_in = fopen(path, "r");
    if (!yr_in) {
        perror(path);
        return 2;
    }
    // Puts results in g_rules if the parse is successful
    if (yr_parse() != 0) {
        return 1;
    }
    return 0;
}
