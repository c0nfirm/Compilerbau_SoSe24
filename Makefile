#!/usr/bin/make
.SUFFIXES:
.PHONY: all run test clean pack docs
.SILENT: run test

TAR = syntree
PCK = abgabe.zip
LIB = libcalc.a

CFLAGS = -std=c11 -Wall -pedantic -MMD -MP -I "src/lib"
AFLAGS = rcs

# collect all of the files for the library archive
LIB_SRC = $(wildcard src/lib/*.c)
LIB_OBJ = $(LIB_SRC:%.c=%.o)
LIB_DEP = $(LIB_OBJ:%.o=%.d)

# collect all of the files for the main binary
TAR_SRC = $(wildcard src/*.c)
TAR_OBJ = $(TAR_SRC:%.c=%.o)
TAR_DEP = $(TAR_SRC:%.c=%.d)

# collect all of the files for the test harness
TST_SRC = $(wildcard tests/*.c)
TST_OBJ = $(TST_SRC:%.c=%.o)
TST_DEP = $(TST_SRC:%.c=%.d)

# combine the files and include make dependencies
SRC = $(LIB_SRC) $(TAR_SRC) $(TST_SRC)
OBJ = $(LIB_OBJ) $(TAR_OBJ) $(TST_OBJ)
DEP = $(LIB_DEP) $(TAR_DEP) $(TST_DEP)
-include $(DEP)

# wildcard rule for compiling a single source file
%.o: %.c
	$(CC) $(CFLAGS) $< -c -o $@

# rule for the shared library
$(LIB): $(LIB_OBJ)
	$(AR) $(AFLAGS) $@ $^

# rule for the main binary
$(TAR): $(TAR_OBJ) $(LIB)
	$(CC) $(CFLAGS) $^ -o $@

# rule for the test harness
harness: $(TST_OBJ) $(LIB)
	$(CC) $(CFLAGS) $^ -o $@


all: $(TAR)

run: all
	./$(TAR)

test: CFLAGS += -DTEST
test: clean harness
	./harness

pack:
	zip -vr $(PCK) src -i "*.c" -i "*.h"

docs: $(SRC)
	doxygen Doxyfile

clean:
	$(RM) $(RMFILES) $(TAR) $(PCK) $(LIB) harness $(OBJ) $(DEP)
