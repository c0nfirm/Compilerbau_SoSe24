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
	switch (result.tag) {
	case PARSE_OK:
		break;
		
	case PARSE_ERR_SYNTAX:
		printf(
			"[x] syntax\n"
			"%s\n",
			result.err
		);
		return EXIT_FAILURE;
		
	case PARSE_ERR_SEMANTIC:
		printf(
			"[✓] syntax\n"
			"[x] analysis\n"
			"%s\n",
			result.err
		);
		return EXIT_FAILURE;
	}
	
	printf(
		"[✓] syntax\n"
		"[✓] analysis\n"
	);
	
	SymDefTable defs = symDefTableNew(&result.tab, &result.ok);
	
	interpret(&result.ok, &defs);
	printf("[✓] interpretation\n");
	
	astProgramRelease(&result.ok);
	symDefTableRelease(&defs);
	return EXIT_SUCCESS;
}
