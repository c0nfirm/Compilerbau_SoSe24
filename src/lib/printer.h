/***************************************************************************//**
 * @file printer.h
 * 
 * This header provides a Printer struct that formats and prints parse trees
 * represented by the Root, Stmt, and Expr types.
 *
 * ## Functionality
 *
 * The Printer is designed to convert parsed expressions and statements into
 * a human-readable string format. It constructs a formatted string by visiting
 * each node in the parse tree and producing an appropriate string
 * representation, including handling nested expressions and variable
 * assignments.
 *
 * ## Usage
 *
 * Create an instance of Printer, pointing it at a file stream , then use it to
 * format a parse tree by passing it together with a Root object to the
 * `printerFormat` function. The function will visit each node in the tree and
 * accumulate the formatted output.
 *
 * ## Examples
 *
 * Basic usage:
 * ```
 * Root root;
 * Printer printer = { .out = stdout };
 * 
 * rootFromStr(&root, "a 3 5 + =");
 * printerFormat(&printer, &root); // prints a=(3+5)
 * 
 * rootRelease(&root);
 * ```
 ******************************************************************************/

#ifndef PRINTER_H_INCLUDED
#define PRINTER_H_INCLUDED

#include <stdio.h>
#include "parse_tree.h"

/* *** structures ********************************************************** */

/**
 * Printer is a struct used for formatting parse trees into human-readable
 * strings.
 *
 * The struct maintains a file stream pointing at the output. It is designed to
 * work by visiting each node in the parse tree (Root, Stmt, Expr) and
 * writing it into the output stream in infix notation. This is particularly
 * useful for debugging or displaying parsed expressions in a clear, textual
 * format.
 *
 * ## Usage
```
	Root root;
	Printer printer = { .out = stdout };
	rootFromStr(&root, "1 2 +");
	printerFormat(&printer, &root);
	rootRelease(&root);
```
 */
typedef struct Printer {
	FILE *out;
} Printer;

/* *** public interface **************************************************** */

/**
 * Folds the entire parse tree starting from a Root object into a single
 * string which is printed into the file stream pointed to by the Printer.
 *
 * Traverses the tree, visiting each statement and expression to generate
 * formatted strings, and then concatenates these strings into a single
 * result separated by newlines.
 */
extern void printerFormat(Printer *self, const Root *root);

/* *** unit tests *********************************************************** */

#ifdef TEST
#include <stdbool.h>

#define PRINTER_TESTS \
	X(printer_add)    \
	X(printer_sub)    \
	X(printer_mul)    \
	X(printer_div)    \
	X(printer_set)

#define X(TEST) extern bool TEST(void);
PRINTER_TESTS
#undef X

#endif /* TEST */
#endif /* PRINTER_H_INCLUDED */
