/***************************************************************************//**
 * @file calculator.h
 * 
 * This header file provides the Calculator struct, which is designed to
 * evaluate arithmetic expressions parsed into a Root structure from a string
 * representation.
 * 
 * It supports operations including addition, subtraction, multiplication,
 * and division, along with variable assignments from 'a' to 'z'.
 *
 * ## Example
 * ```
 * Root root;
 * Calculator calculator;
 * 
 * rootFromStr(&root, "a 1 = b 2 3 * = a b +");
 * int result = calculatorCalc(&calculator, &root);
 * printf("Final result: %i\n", result); // prints 7
 * rootRelease(&root);
 * ```
 ******************************************************************************/

#ifndef CALCULATOR_H_INCLUDED
#define CALCULATOR_H_INCLUDED

#include "parse_tree.h"

/* *** structures *********************************************************** */

/**
 * Calculator is a struct designed to evaluate parsed arithmetic expressions.
 *
 * ## Usage
```
	Root root;
	Calculator calculator;
	rootFromStr(&root, "1 2 +");
	int result = calculatorCalc(&calculator, &root);
	printf("The result of the calculation is: %i\n", result);
	rootRelease(&root);
```
 */
typedef struct Calculator {
	// TODO: Attribute für Variablenbelegungen und Ergebnis fehlen
} Calculator;

/* *** public interface ***************************************************** */

/**
 * Evaluates the entire parse tree starting from a `Root` and returns the
 * result of the last expression evaluated.
 */
extern int calculatorCalc(Calculator *self, Root *root);

/* *** unit tests *********************************************************** */

#ifdef TEST
#include <stdbool.h>

#define CALCULATOR_TESTS \
	X(calc_add)          \
	X(calc_sub)          \
	X(calc_mul)          \
	X(calc_div)          \
	X(calc_set)          \
	X(calc_vars)

#define X(TEST) extern bool TEST(void);
CALCULATOR_TESTS
#undef X

#endif /* TEST */
#endif /* CALCULATOR_H_INCLUDED */
