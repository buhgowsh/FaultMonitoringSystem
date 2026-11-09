CC ?= gcc
CFLAGS += -std=c11 -Wall -Wextra -g -Isrc
SAN = -fsanitize=address,undefined -fno-omit-frame-pointer
CMOCKA_LIBS := $(shell pkg-config --libs cmocka)

TEST_SRCS := $(wildcard tests/test_*.c)
TESTS := $(TEST_SRCS:.c=)

.PHONY: test clean
test: $(TESTS)
	@for t in $(TESTS); do echo "== $$t"; ./$$t || exit 1; done

tests/test_%: tests/test_%.c
	$(CC) $(CFLAGS) $(SAN) $< $(wildcard src/$*.c) -o $@ $(CMOCKA_LIBS) $(WRAP_test_$*)

clean:
	rm -f $(TESTS)

all:
	cd parsers; $(MAKE)
