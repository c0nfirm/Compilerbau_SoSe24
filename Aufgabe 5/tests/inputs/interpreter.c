#include <stdio.h>
#include <stdlib.h>

#include <parser.tab.h>
#include <symtab.h>
#include <ast.h>
#include <interpreter.h>

const int SEMANTIC_CHECK = 1;

int main(int argc, const char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <c1-source>\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	FILE *in = fopen(argv[1], "r");
	if (in == NULL) {
		fprintf(stderr, "Failed to read c1 source file\n");
		return EXIT_FAILURE;
	}
	
	ParseResult result = astParse(in);
	SymDefTable defs;
	switch (result.tag) {
	case PARSE_OK:
		defs = symDefTableNew(&result.tab, &result.ok);
		interpret(&result.ok, &defs);
		astProgramRelease(&result.ok);
		symDefTableRelease(&defs);
		break;
		
	default:
		puts(result.err);
		break;
	}
	
	return result.tag;
}
