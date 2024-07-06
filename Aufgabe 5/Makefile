#!/usr/bin/make
.SUFFIXES:
.PHONY: all run test bless clean pack docs

TAR = minako
PCK = abgabe.zip

export LIB = libminako.a
export ROOT_DIR = $(CURDIR)
LIB_PATH = src/lib

export CFLAGS = -std=c11 -g -Wall -pedantic -MMD -MP -I $(ROOT_DIR)/$(LIB_PATH)
AFLAGS = rcs

# collect all of the files for the library archive
LIB_LEX = $(wildcard $(LIB_PATH)/*.l)
LIB_YAC = $(wildcard $(LIB_PATH)/*.y)
LIB_SRC = $(wildcard $(LIB_PATH)/*.c)
LIB_OBJ = $(LIB_YAC:%.y=%.tab.o) $(LIB_LEX:%.l=%.o) $(LIB_SRC:%.c=%.o)
LIB_DEP = $(LIB_OBJ:%.o=%.d)

# collect all of the files for the main binary
TAR_SRC = $(wildcard src/*.c)
TAR_OBJ = $(TAR_SRC:%.c=%.o)
TAR_DEP = $(TAR_SRC:%.c=%.d)

# combine the files and include make dependencies
SRC = $(LIB_SRC) $(TAR_SRC)
OBJ = $(LIB_OBJ) $(TAR_OBJ)
DEP = $(LIB_DEP) $(TAR_DEP)
-include $(DEP)

ifneq ($(OS),Windows_NT)
# select gm4 instead of m4 for OSX which is currently broken
	ifeq ($(shell uname -s),Darwin)
		export M4 = gm4
	endif
# enable the address-sanitizer; msys2 gcc doesn't support it
	export LDFLAGS = -fsanitize=address,undefined
	export ASAN_OPTIONS = detect_leaks=0
	CFLAGS += -fsanitize=address,undefined
endif

# rule for the flex lexer
%.c: %.l
	flex -t $< > $@

# rule for the bison parser
%.tab.c: %.y
	bison -d -o $@ -v $<

# rule for compiling a single source file
%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

# rule for the shared library
$(LIB): $(LIB_OBJ)
	$(AR) $(AFLAGS) $@ $^

# rule for the main binary
$(TAR): $(LIB) $(TAR_OBJ)
	$(CC) $(LDFLAGS) $(TAR_OBJ) $(LIB) -o $@

all: $(TAR)

run: all
	./$(TAR) tests/inputs/ok/simple.c1

test: $(LIB)
	$(MAKE) -sC tests unit suite

bless: $(LIB)
	$(MAKE) -sC tests bless

pack:
	zip -vr $(PCK) src -i "*.c" -i "*.h" -i "*.l" -i "*.y" -x $(LIB_LEX:%.l=%.c) -x $(LIB_YAC:%.y=%.tab.c) -x $(LIB_YAC:%.y=%.tab.h)

docs: $(SRC)
	doxygen Doxyfile

clean:
	@$(RM) $(RMFILES) $(TAR) $(PCK) $(LIB) $(OBJ) $(DEP) $(LIB_LEX:%.l=%.c) $(LIB_YAC:%.y=%.tab.c) $(LIB_YAC:%.y=%.tab.h) $(LIB_YAC:%.y=%.output)
	@$(MAKE) -sC tests clean
