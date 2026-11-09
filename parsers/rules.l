%option noyywrap yylineno caseless nounput noinput
%option outfile = "rules_lexer.c"

%{

#include "rules.tab.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

%}

WORD       \[[a-z0-9._-]+\]
NUMBER     [0-9]+
WHITESPACE [ \v\t\r\f\n]+

%%

{WHITESPACE}            ; // ignore whitespace
['"][^\n]*              ; // comments run to next newline

"<="                    { yr_lval.cmp = CMP_LE; return OP; }
">="                    { yr_lval.cmp = CMP_GE; return OP; }
"<"                     { yr_lval.cmp = CMP_LT; return OP; }
">"                     { yr_lval.cmp = CMP_GT; return OP; }
"%"                     return PERCENT;

if                      return IF;
then                    return THEN;
and                     return AND;
halt                    return HALT;
log                     return LOG;
cpu                     return CPU;
ram|vram                {
                            yr_lval.res = (tolower((unsigned char)yr_text[0]) == 'v')
                                         ? RES_VRAM : RES_RAM;
                            return RAM;
                        }
mb|gb                   {
                            // value = number of MB in one unit
                            yr_lval.dbl = (tolower((unsigned char)yr_text[0]) == 'g')
                                         ? 1024.0 : 1.0;
                            return SIZE;
                        }
priority                return PRIORITY;
inc|incr|incre|increm|increme|incremen|increment    return INC;
dec|decr|decre|decrem|decreme|decremen|decrement    return DEC;

{WORD}                  {
                            // TODO depending on implementation, converting to lowercase
                            //   might be unecessary; use below if so
                            //yr_lval.str = strndup(yr_text + 1, yr_leng - 2);
                            // Get rid of the brackets and convert to lowercase
                            const int len = strlen(yr_text) - 1; // -2 brackets +1 null byte
                            char* ret = malloc(len * sizeof(char));
                            for (int i = 0; i < len - 1; i++) {
                                ret[i] = tolower(yr_text[i+1]);
                            }
                            ret[len-1] = '\0'; // Explicity set the null byte just in case
                            //printf("%d\n", len);
                            //for (int i = 0; i < len; i++) {
                            //    printf("%d %X\n", i, ret[i]);
                            //}
                            yr_lval.str = ret;

                            return WORD;
                        }

{NUMBER}                {
                            yr_lval.num = strtol(yr_text, NULL, 10);
                            return NUMBER;
                        }

.   { return yr_text[0]; /* unknown character: parser reports the error */ }

%%
