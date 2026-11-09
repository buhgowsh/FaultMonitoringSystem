%option noyywrap yylineno caseless nounput noinput
%option outfile = "config_lexer.c"

%{

#include "config.tab.h"

%}

NUMBER  [0-9]+ 
WHITESPACE [ \v\t\r\f\n]+

%%

{WHITESPACE}            ; // ignore whitespace
['"][^\n]*              ; // comments run to next newline


"="                     return ASSIGN;
log_splits              return LOGSPLIT;
never                   return NEVER;
daily                   return DAILY;
weekly                  return WEEKLY;
monthly                 return MONTHLY;
max_logs                return MAXLOG;
halt_sound              return HALTSOUND;
none                    return NONE;
bell                    return BELL;
log_sound               return LOGSOUND;
increase_priority_sound return INCSOUND;
decrease_priority_sound return DECSOUND;
log_open                return LOGOPEN;
always                  return ALWAYS;
log_close               return LOGCLOSE;
crashes                 return CRASH;
halt_warning            return HALTWARN;

{NUMBER}                {
                            yc_lval.num = strtol(yc_text, NULL, 10);
                            return NUMBER;
                        }

.   { return yc_text[0]; /* unknown character: parser reports the error */ }

%%
