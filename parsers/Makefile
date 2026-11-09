RULES-OBJS= rules.tab.c rules_lexer.c rules.h
CONFIG-OBJS= config.tab.c config_lexer.c config.h

parser: main.c $(RULES-OBJS)
	$(CC) -o $@ main.c rules.tab.c rules_lexer.c

clean: clean-rules clean-config
	rm -f parser

##-- Rule Parser --##
rules.tab.c rules.tab.h: rules.y rules.h
	bison -p "yr_" -Wall -d rules.y

rules_lexer.c: rules.l rules.tab.h
	flex -P "yr_" rules.l

clean-rules:
	rm -f rules.tab.c rules.tab.h rules_lexer.c

##-- Config Parser --##
config.tab.c config.tab.h: config.y config.h
	bison -p "yc_" -Wall -d config.y

lex.yc_.c: config.l config.tab.h
	flex -P "yc_" config.l

clean-config:
	rm -f config.tab.c config.tab.h config_lexer.c

##-- Testing --##
# TODO automate testing examples
test-rules: rules
	./parser tests/in1.txt
	./parser tests/in2.txt
	./parser tests/in3.txt
	./parser tests/in4.txt
	./parser tests/in5.txt

.PHONY: clean-rules clean-config clean test-rules
