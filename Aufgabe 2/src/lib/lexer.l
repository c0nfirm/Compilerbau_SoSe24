%option noyywrap nounput noinput nodefault
%option never-interactive yylineno

WHITESPACE [[:space:]]

digit	[0-9]
INT		[[:digit:]]+
FLT		{INT}"."{INT}|"."[INT]

%{
	#include <stdlib.h>
	#include "parser.tab.h"
%}

%%

{WHITESPACE}+ { /* ignore whitespaces */ }

 [/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]       { /* ignores blok comment */ }
 [/][*]                                    { fatal_error("Unterminated comment"); }

 "//".*			{ /* ignore line comment */ }

 "=="	return EQ;
 "!="	return NEQ;
 "<"	return LT;
 ">"	return GT;
 "<="	return LEQ;
 ">="	return GEQ;

 "+"	return ADD;
 "-"	return SUB;
 "*"	return MUL;
 "/"	return DIV;
 
 "="	return ASSIGN;
 
 "do"		return KW_DO;
 "else"		return KW_ELSE;
 "for"		return KW_FOR;
 "if"		return KW_IF;
 "print"	return KW_PRINT;
 "return"	return KW_RETURN;
 "while"	return KW_WHILE;
 
 {FLT}([eE][-+]?{INT})? |
 {INT}([eE][-+]?[INT])	{yylval.floatValue = strtod(yytext, NULL); return FLOAT_LITERAL;}

 {INT}	{yylval.floatValue = strtol(yytext, NULL); return FLOAT_LITERAL;}

 "true"		{yylval.intValue = 1; return BOOL_LITEAL;}
 "false"	{yylval.intValue = 0; return BOOL_LITERAL;}

 /* TODO: Stringliterale */
 \"[^\n\"]*\"{
	yylval.string = malloc(yylen + 1)
 }

 /* TODO: Bezeichner */

 /* TODO: Separatoren */

.             return YYUNDEF;
<<EOF>>       return EOF;

%%

/* function needed during testing, please do not remove */
void lexer_reset_state(void) {
	BEGIN(INITIAL);
}
