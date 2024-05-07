#!/usr/bin/make
.SUFFIXES:
.PHONY: all unit suite clean

TAR = harness
LIB = ../libminako.a

CFLAGS = -std=c11 -g -Wall -pedantic -MMD -MP -I "../src/lib"

# collect all of the files for the test harness
SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)
DEP = $(SRC:%.c=%.d)
-include $(DEP)

# collect all of the testsuite programs
BLESS_SRC = inputs/lexer.c
BLESS_OBJ = $(BLESS_SRC:%.c=%.o)
BLESS_TAR = $(BLESS_SRC:%.c=%)
BLESS_DEP = $(BLESS_SRC:%.c=%.d)
-include $(BLESS_DEP)

# collect all of the files for the input tests
OK_SRC    = $(wildcard inputs/ok/*.c1)
LEX_SRC   = $(wildcard inputs/lexical_err/*.c1)
SUITE_SRC = $(OK_SRC) $(LEX_SRC)

SUITE_TOK = $(SUITE_SRC:%.c1=%.token)
SUITE_TOK_DIFF = $(SUITE_SRC:%.c1=%.token_diff)

# rule for compiling a single source file
%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

%.token: %.c1 inputs/lexer
	./inputs/lexer $< > $@

%.token_diff: %.c1
	./inputs/lexer $< | diff -s - $*.token > $@

inputs/%: inputs/%.o
	$(CC) $< $(LIB) -o $@

# rule for the test harness for the unit-tests
$(TAR): $(LIB) $(OBJ)
	$(CC) $(OBJ) $(LIB) -o $@

all: $(TAR)

unit: all
	./$(TAR)

suite: $(BLESS_TAR) $(SUITE_SRC:%.c1=%.token_diff)
	$(RM) $(RMFILES) $^

clean:
	$(RM) $(RMFILES) $(TAR) $(OBJ) $(DEP) $(BLESS_TAR) $(BLESS_OBJ) $(BLESS_DEP) $(SUITE_TOK_DIFF)
