#ifndef RULE_ENGINE_H
#define RULE_ENGINE_H

#include "readproc.h"
#include "parsers/rules.h"

typedef enum {
    RULE_NO_MATCH,
    RULE_MATCH,
    RULE_UNAVAILABLE,
    RULE_INVALID
} RuleResult;

/* ram_total_mib must use the same total used by the parser. CPU retains
 * the collector's 100% per logical core scale. VRAM is unavailable. */
RuleResult rule_matches_process(const Rule *rule, const ProcessInfo *process,
                                double ram_total_mib);

/* Called for EVERY action of EVERY matching process, in rule/action order.
 * The callback borrows these pointers for the duration of the call only.
 * No OS actions or repetition policy are implemented by this evaluator. */
typedef void (*RuleActionVisitor)(size_t rule_index, const ProcessInfo *process,
                                  const Action *action, void *context);
size_t rule_engine_visit(const Rule *rules, int rule_count,
                      const ProcessSnapshot *snapshot, double ram_total_mib,
                      RuleActionVisitor visit, void *context);

/* Report-only callback: context is FILE*. Prints intended actions only. */
void report_rule_action(size_t rule_index, const ProcessInfo *process,
                        const Action *action, void *context);

/* Main-loop entry point. Uses g_rules, g_n_rules and g_total_ram_mb supplied
 * by the existing parser. Call only after successful parser startup and CPU
 * calculation. Reports intended actions on every matching sample; no process
 * modifications. Returns the number of matching rule/process pairs. */
size_t rule_engine_evaluate(const ProcessSnapshot *snapshot);


// Call once at startup, before evaluating any process snapshots.
int rule_engine_init(int argc, char **argv);

#endif
