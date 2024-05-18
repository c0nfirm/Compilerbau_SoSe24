%option noyywrap nounput noinput nodefault
%option never-interactive yylineno

WHITESPACE [[:space:]]

digit	[0-9]
INT		[[:digit:]]+
FLT		{INT}"."{INT}|"."{INT}
ASCII	[,|;|(|)|{|}]

%{
	#include <stdlib.h>
	#include "parser.tab.h"
%}

%%

{WHITESPACE}+ { /* ignore whitespaces */ }

 [/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]       { /* ignores blok comment */ }
 [/][*]                                    { yy_fatal_error("Unterminated comment"); }

 "//".*									   { /* ignore line comment */ }

 "=="	return EQ;
 "!="	return NEQ;
 "<"	return LT;
 ">"	return GT;
 "<="	return LEQ;
 ">="	return GEQ;

 [&&]	return LOG_AND;
 "||"	return LOG_OR;

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

 "int"		return KW_INT;
 "bool"		return KW_BOOLEAN;
 "float"	return KW_FLOAT;
 "void"		return KW_VOID;
 
 {FLT}([eE][-+]?{INT})? |
 {INT}([eE][-+]?{INT})	{yylval.floatValue = strtod(yytext, NULL); return FLOAT_LITERAL;}

 {INT}	{yylval.intValue = strtol(yytext, NULL, 10); return INT_LITERAL;}

 "true"		{yylval.intValue = 1; return BOOL_LITERAL;}
 "false"	{yylval.intValue = 0; return BOOL_LITERAL;}

\"(\\.|[^\\"])*\" {
	yylval.string = malloc(yyleng -1 + 2);
	memcpy(yylval.string, yytext + 1, yyleng - 2);
	yylval.string[yyleng - 2] = 0;
	return STRING_LITERAL;
 }

 [[:alpha:]][[:alnum:]_]* {
	yylval.string = malloc(yyleng + 1);
	strcpy(yylval.string, yytext);
	return IDENT;
}

 {ASCII}	{return (int)yytext[0];}

.             return YYUNDEF;
<<EOF>>       return EOF;

%%

/* function needed during testing, please do not remove */
void lexer_reset_state(void) {
	BEGIN(INITIAL);
}
