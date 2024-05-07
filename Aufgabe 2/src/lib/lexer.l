%option noyywrap nounput noinput nodefault
%option never-interactive yylineno

WHITESPACE [[:space:]]

%{
	#include <stdlib.h>
	#include "parser.tab.h"
%}

%%

{WHITESPACE}+ { /* ignore whitespaces */ }

 /* TODO: Blockkommentar */
 /* TODO: Zeilenkommentar */

 /* TODO: Vergleichsoperatoren */
 /* TODO: arithmetische Operatoren */
 /* TODO: Zuweisungsoperator */
 
 /* TODO: Schlüsselworte */
 
 /* TODO: Fließkommaliterale */
 /* TODO: Integerliterale */
 /* TODO: boolesche Literale */
 /* TODO: Stringliterale */
 /* TODO: Bezeichner */

 /* TODO: Separatoren */

.             return YYUNDEF;
<<EOF>>       return EOF;

%%

/* function needed during testing, please do not remove */
void lexer_reset_state(void) {
	BEGIN(INITIAL);
}
