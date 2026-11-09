RULES-OBJS= rules.tab.c rules_lexer.c
CONFIG-OBJS= config.tab.c config_lexer.c

parser: main.c $(RULES-OBJS) rules.h $(CONFIG-OBJS) config.h
	$(CC) -o $@ main.c $(RULES-OBJS) $(CONFIG-OBJS)

clean: clean-rules clean-config
	rm -f parser

##-- Rule Parser --##
rules.tab.c rules.tab.h: rules.y rules.h
	bison -p "yr_" -Wall -d rules.y

rules_lexer.c: rules.l rules.tab.h
	flex -P "yr_" rules.l

clean-rules:
	rm -f rules.tab.c rules.tab.h rules_lexer.c

##-- Config 	Parser --##
config: config.tab.c config_lexer.c config.h
	$(CC) -o $@ config.tab.c config_lexer.c

config.tab.c config.tab.h: config.y config.h
	bison -p "yc_" -Wall -d config.y

config_lexer.c: config.l config.tab.h
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
