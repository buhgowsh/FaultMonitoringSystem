#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { LS_NEVER, LS_DAILY, LS_WEEKLY, LS_MONTHLY } LogSplitOpts;
typedef enum { LO_NEVER, LO_ALWAYS } LogOpenOpts;
typedef enum { LC_NEVER, LC_CRASH, LC_ALWAYS } LogCloseOpts;
typedef enum { S_NONE, S_BELL } SoundOpts;

typedef struct {
	LogSplitOpts logsplit;
	unsigned int maxlog;

	LogOpenOpts logopen;
	LogCloseOpts logclose;

	unsigned int haltwarn;

	SoundOpts logsound;
	SoundOpts haltsound;
	SoundOpts incsound;
	SoundOpts decsound;
} Config;

// Result of a successful parse
extern Config g_config;

int config_parse(const char *path);

#endif /* CONFIG_H */
