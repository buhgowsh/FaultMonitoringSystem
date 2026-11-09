%code requires {
#include "config.h"
}

%{

#include "config.h"

// Must define these elso error is thrown by the compiler
int yc_lex(void);
void yc_error(const char *msg);
extern int yc_lineno;
extern FILE *yc_in;

// Defaults
Config g_config = { 
    .logsplit =  LS_NEVER,
    .maxlog =    1,
    .logopen =   LO_NEVER,
    .logclose =  LC_CRASH,
    .haltwarn =  0,
    .logsound =  S_NONE,
    .haltsound = S_NONE,
    .incsound =  S_NONE,
    .decsound =  S_NONE,
};

// Out of memory; hopefully shouldn't happen
static void oom(void) {
    perror("memory");
    exit(3);
}

%}

%define parse.error verbose

%union {
    unsigned int num;
    LogSplitOpts logsplit;
    LogOpenOpts  logopen;
    LogCloseOpts logclose;
    SoundOpts    sound;
}

%token <num> NUMBER
%token <sound> BELL
%token <sound> NONE
%token <logsplit> DAILY
%token <logsplit> WEEKLY
%token <logsplit> MONTHLY
%token <logclose> CRASH
%token NEVER ALWAYS

%token LOGSPLIT MAXLOG HALTSOUND LOGSOUND INCSOUND DECSOUND LOGOPEN LOGCLOSE HALTWARN ASSIGN

%type <logsplit> logsplitopts
%type <logopen>  logopenopts
%type <logclose> logcloseopts
%type <sound>    logsoundopts
%type <sound>    haltsoundopts
%type <sound>    incsoundopts
%type <sound>    decsoundopts

%start config

%%

config
    : %empty
    | config item
    ;

item
    : LOGSPLIT  ASSIGN logsplitopts
    | MAXLOG    ASSIGN maxlogopts
    | LOGOPEN   ASSIGN logopenopts
    | LOGCLOSE  ASSIGN logcloseopts
    | HALTWARN  ASSIGN haltwarnopts
    | LOGSOUND  ASSIGN logsoundopts
    | HALTSOUND ASSIGN haltsoundopts
    | INCSOUND  ASSIGN incsoundopts
    | DECSOUND  ASSIGN decsoundopts
    ;

logsplitopts
    : NEVER                 { g_config.logsplit = LS_NEVER; }
    | DAILY                 { g_config.logsplit = LS_DAILY; } 
    | WEEKLY                { g_config.logsplit = LS_WEEKLY; } 
    | MONTHLY               { g_config.logsplit = LS_MONTHLY; } 
    ;

maxlogopts
    : NUMBER                { g_config.maxlog = $1; }
    ;

logopenopts
    : NEVER                 { g_config.logopen = LO_NEVER; }
    | ALWAYS                { g_config.logopen = LO_ALWAYS; }
    ;

logcloseopts
    : CRASH                 { g_config.logclose = LC_CRASH; }
    | NEVER                 { g_config.logclose = LC_NEVER; }
    | ALWAYS                { g_config.logclose = LC_ALWAYS; }
    ;

haltwarnopts
    : NUMBER                    { g_config.haltwarn = $1; }
    ;
logsoundopts
    : NONE                      { g_config.logsound = S_NONE; }
    | BELL                      { g_config.logsound = S_BELL; }
    ;
haltsoundopts
    : NONE                      { g_config.haltsound = S_NONE; }
    | BELL                      { g_config.haltsound = S_BELL; }
    ;
incsoundopts
    : NONE                      { g_config.incsound = S_NONE; }
    | BELL                      { g_config.incsound = S_BELL; }
    ;
decsoundopts
    : NONE                      { g_config.decsound = S_NONE; }
    | BELL                      { g_config.decsound = S_BELL; }
    ;

%%

void yc_error(const char *msg) {
    fprintf(stderr, "line %d: %s\n", yc_lineno, msg);
}

int config_parse(const char *path) {
    if (!path) {    // COPIED from parsers/rules.y
        // Get path to home dir
        char* home = getenv("HOME");
        if (!home) { oom(); }
        // TODO discuss name and file extension for rule file
        path = "/.config/fms/config";

        // TODO could make more efficient, but cannot cat directly into home
        // Make a string big enough to support the concat of home and path
        int len = strlen(home) + strlen(path) + 1;
        char* default_path = malloc(len * sizeof(char));
        if (!default_path) { oom(); }
        default_path[0] = '\0';     // Makes sure strcat works correctly
        default_path[len-1] = '\0'; // strcat doesn't add null byte (I believe)

        default_path = strcat(default_path, home);
        path = strcat(default_path, path);
    }
    yc_in = fopen(path, "r");
    if (!yc_in) {
        perror(path);
        return 2;
    }
    // Puts results in g_config if the parse is successful
    if (yc_parse() != 0) {
        return 1;
    }
    return 0;
}
