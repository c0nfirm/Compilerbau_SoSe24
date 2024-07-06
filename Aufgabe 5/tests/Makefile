#!/usr/bin/make
.SUFFIXES:
.PHONY: unit suite bless clean suite_lexer suite_parser suite_analyzer suite_interpreter

# unit-test harness
UNIT_TAR = harness
UNIT_SRC = $(wildcard *.c)
UNIT_OBJ = $(UNIT_SRC:%.c=%.o)
UNIT_DEP = $(UNIT_SRC:%.c=%.d)

# integration-test runners
BLESS_SRC = $(wildcard inputs/*.c)
BLESS_OBJ = $(BLESS_SRC:%.c=%.o)
BLESS_DEP = $(BLESS_SRC:%.c=%.d)
BLESS_TAR = $(BLESS_SRC:%.c=%)

# include the dependency rules
-include $(UNIT_DEP) $(BLESS_DEP)

# collect all of the files for the input tests
OK_SRC    = $(wildcard inputs/ok/*.c1)
SYN_SRC   = $(wildcard inputs/parser_err/*.c1)
SEM_SRC   = $(wildcard inputs/analysis_err/*.c1)
RUN_SRC   = $(wildcard inputs/interpreter_err/*.c1)

# collect the correct c1-programs for the compiler phases
SUITE_LEX = $(OK_SRC:%.c1=%.token) $(SYN_SRC:%.c1=%.token) $(SEM_SRC:%.c1=%.token) $(RUN_SRC:%.c1=%.token)
SUITE_SYN = $(OK_SRC:%.c1=%.ast) $(SEM_SRC:%.c1=%.ast) $(RUN_SRC:%.c1=%.ast)
SUITE_SEM = $(OK_SRC:%.c1=%.ast-resolved) $(RUN_SRC:%.c1=%.ast-resolved)
SUITE_RUN = $(OK_SRC:%.c1=%.output)

# collect the incorrect c1-programs for the compiler phases
SUITE_SYN_ERR = $(SYN_SRC:%.c1=%.syn_err)
SUITE_SEM_ERR = $(SEM_SRC:%.c1=%.sem_err)
SUITE_RUN_ERR = $(RUN_SRC:%.c1=%.run_err)

SUITE_LEX_DIFF = $(SUITE_LEX:%.token=%.lex_diff)
SUITE_SYN_DIFF = $(SUITE_SYN:%.ast=%.syn_diff)
SUITE_SEM_DIFF = $(SUITE_SEM:%.ast-resolved=%.sem_diff)
SUITE_RUN_DIFF = $(SUITE_RUN:%.output=%.run_diff)

# terminal output for ok and error results
OK  = \033[32m ok\033[0m
ERR = \033[1;31merr\033[0m
SEM = \033[33msem\033[0m

# rule for compiling a single source file
%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

# rules for generating the reference output
%.token: %.c1 inputs/lexer
	@./inputs/lexer $< > $@

%.ast: %.c1 inputs/parser
	@./inputs/parser $< > $@

%.ast-resolved: %.c1 inputs/analyzer
	@./inputs/analyzer $< > $@

%.output: %.c1 inputs/interpreter
	@./inputs/interpreter $< > $@

# prints lexer token into the standard output and diffs that with the reference output
%.lex_diff: %.c1 inputs/lexer
	@./inputs/lexer $< 2>&1 | diff -bc - $*.token > $@ && $(RM) $(RMFILES) $@ && printf "[$(OK)] $*\n" || printf "[$(ERR)] $*: diff in $@\n"

# prints the resolved abstract syntax tree and diffs that with the reference output
%.sem_diff: %.c1 inputs/analyzer
	@./inputs/analyzer $< 2>&1 | diff -bc - $*.ast-resolved > $@ && $(RM) $(RMFILES) $@ && printf "[$(OK)] $*\n" || printf "[$(ERR)] $*: diff in $@\n"

# prints the program output and diffs that with the reference output
%.run_diff: %.c1 inputs/interpreter
	@./inputs/interpreter $< 2>&1 | diff -bc - $*.output > $@ && $(RM) $(RMFILES) $@ && printf "[$(OK)] $*\n" || printf "[$(ERR)] $*: diff in $@\n"

# runs the parser with debug output and creates the file if parsing failed
%.syn_diff: %.c1 inputs/parser
	@./inputs/parser $< > $*.syn 2>&1; ec=$$?; \
	if [ $$ec -eq 0 ]; then \
		diff -bc $*.syn $*.ast > $@ && printf "[$(OK)] $*\n" && $(RM) $(RMFILES) $@ || printf "[$(ERR)] $*: trace in $@\n"; \
	elif [ $$ec -eq 1 ]; then \
		printf "[$(ERR)] $*: " | cat - $*.syn; \
	elif [ $$ec -eq 2 ]; then \
		printf "[$(SEM)] $*: " | cat - $*.syn; \
	fi
	@$(RM) $(RMFILES) $*.syn

# runs the parser with debug output and creates the file if parsing was successful
%.syn_err: %.c1 inputs/parser
	@./inputs/parser $< > $@ 2>&1; ec=$$?; \
	if [ $$ec -ne 1 ]; then \
		printf "[$(ERR)] $*\n"; \
	else \
		printf "[$(OK)] $*: " | cat - $@; \
	fi
	$(RM) $(RMFILES) $@

# runs the semantic analyzer and prints the error message in case it successfully fails
%.sem_err: %.c1 inputs/analyzer
	@./inputs/analyzer $< > $@ 2>&1; ec=$$?; \
	if [ $$ec -ne 2 ]; then \
		printf "[$(ERR)] $*\n"; \
	else \
		printf "[$(OK)] $*: " | cat - $@; \
	fi
	$(RM) $(RMFILES) $@

# runs the interpreter and prints an error message if undefined behavior is detected
%.run_err: %.c1 inputs/interpreter
	@./inputs/interpreter $< > $@ 2>&1; ec=$$?; \
	if [ $$ec -ne 3 ]; then \
		printf "[$(ERR)] $*\n"; \
	else \
		printf "[$(OK)] $*: " | cat - $@; \
	fi
	$(RM) $(RMFILES) $@

# generic rule for the integration-test runners
inputs/%: inputs/%.o $(ROOT_DIR)/$(LIB)
	$(CC) $(LDFLAGS) $^ -o $@

# compile the unit test harness
$(UNIT_TAR): $(UNIT_OBJ) $(ROOT_DIR)/$(LIB)
	$(CC) $(LDFLAGS) $^ -o $@

# run the unit tests
unit: $(UNIT_TAR)
	echo "--- [Unit Tests] ---"
	./$(UNIT_TAR)

suite_lexer:
	echo "--- [Lexer Tests] ---"

suite_parser:
	echo "--- [Parser Tests] ---"

suite_analyzer:
	echo "--- [Analyzer Tests] ---"

suite_interpreter:
	echo "--- [Interpreter Tests] ---"

# run the test-suite
suite: suite_lexer $(SUITE_LEX_DIFF) suite_parser $(SUITE_SYN_DIFF) $(SUITE_SYN_ERR) suite_analyzer $(SUITE_SEM_DIFF) $(SUITE_SEM_ERR) suite_interpreter $(SUITE_RUN_DIFF) $(SUITE_RUN_ERR)

# generate the reference output
bless: $(SUITE_LEX) $(SUITE_SYN) $(SUITE_SEM) $(SUITE_RUN)

clean:
	$(RM) $(RMFILES) $(UNIT_TAR) $(UNIT_DEP) $(UNIT_OBJ) $(BLESS_TAR) $(BLESS_DEP) $(BLESS_OBJ) $(SUITE_LEX_DIFF) $(SUITE_SYN_DIFF) $(SUITE_SEM_DIFF) $(SUITE_RUN_DIFF)
