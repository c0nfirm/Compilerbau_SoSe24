/***************************************************************************//**
 * @file harness.c
 * @author Dorian Weber
 * @brief Source file containing the test-harness for unit tests.
 * 
 * This file will be linked instead of `main.c` for make-target `test`.
 ******************************************************************************/

#include <stdio.h>

#include <parse_tree.h>
#include <printer.h>
#include <calculator.h>

// auxiliary code for the test harness - only linked for target 'test'
int main(void) {
	#ifdef TEST
	#define X(CASE) do { \
		fputs(#CASE ":\t", stderr); \
		fputs(CASE() ? "(success)" : "(failure)", stderr); \
	} while (0);
	
	PARSE_TREE_TESTS
	CALCULATOR_TESTS
	PRINTER_TESTS
	
	#undef X
	return 0;
	#else
	#error "The test-harness requires TEST to be defined on the command line"
	#endif
}
