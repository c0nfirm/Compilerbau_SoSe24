/***************************************************************************//**
 * @file main.c
 * Entry point for the program.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <parse_tree.h>
#include <printer.h>
#include <calculator.h>

int main(int argc, const char *argv[]) {
	//read the expression from the command line
	const char *expr = "12+";
	if (argc > 1) { expr = argv[1]; }
	
	// parse out the parse tree
	Root tree;
	switch (rootFromStr(&tree, expr)) {
	case PARSE_ERR_LEXICAL:
		fprintf(stderr, "Lexical error: unexpected character\n");
		exit(-1);
		
	case PARSE_ERR_SYNTAX:
		fprintf(stderr, "Syntax error: wrong number of operands\n");
		exit(-1);
		
	case PARSE_ERR_SEMANTIC:
		fprintf(stderr, "Semantic error: assignment to non-variable\n");
		exit(-1);
		
	case PARSE_OK:
		break;
	}
	
	// use the pretty-printer for output
	Printer printer = { .out = stdout };
	printerFormat(&printer, &tree);
	
	// use the calculator for evaluation
	Calculator calc;
	printf("%i\n", calculatorCalc(&calc, &tree));
	
	// return success
	rootRelease(&tree);
	return 0;
}
