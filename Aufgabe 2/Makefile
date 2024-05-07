#!/usr/bin/make
.SUFFIXES:
.PHONY: all run test clean pack

TAR = minako
PCK = abgabe.zip
LIB = libminako.a

CFLAGS = -std=c11 -g -Wall -pedantic -MMD -MP -I "src/lib"
AFLAGS = rcs

# collect all of the files for the library archive
LIB_LEX = $(wildcard src/lib/*.l)
LIB_SRC = $(wildcard src/lib/*.c)
LIB_OBJ = $(LIB_LEX:%.l=%.o) $(LIB_SRC:%.c=%.o)
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

# rule for the flex lexer
%.c: %.l
	flex -t $< > $@

# rule for compiling a single source file
%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

# rule for the shared library
$(LIB): $(LIB_OBJ)
	$(AR) $(AFLAGS) $@ $^

# rule for the main binary
$(TAR): $(LIB) $(TAR_OBJ)
	$(CC) $(TAR_OBJ) $(LIB) -o $@


all: $(TAR)

run: all
	@./$(TAR) tests/inputs/ok/demorgan.c1

test: $(LIB)
	@$(MAKE) -sC tests unit
	@$(MAKE) -skC tests suite

pack:
	zip -vr $(PCK) src -i "*.c" -i "*.h" -i "*.l" -x $(LIB_LEX:%.l=%.c)

clean:
	@$(RM) $(RMFILES) $(TAR) $(PCK) $(LIB) $(OBJ) $(DEP) $(LIB_LEX:%.l=%.c)
	@$(MAKE) -sC tests clean
