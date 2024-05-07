#ifndef YY_YY_SRC_LIB_PARSER_TAB_H_INCLUDED
#define YY_YY_SRC_LIB_PARSER_TAB_H_INCLUDED

#include <stdio.h>

enum {
    YYUNDEF = 257,        /* "invalid token"  */
    LOG_AND = 258,        /* "&&"  */
    LOG_OR = 259,         /* "||"  */
    EQ = 260,             /* "=="  */
    NEQ = 261,            /* "!="  */
    LT = 262,             /* "<"  */
    GT = 263,             /* ">"  */
    LEQ = 264,            /* "<="  */
    GEQ = 265,            /* ">="  */
    ADD = 266,            /* "+"  */
    SUB = 267,            /* "-"  */
    MUL = 268,            /* "*"  */
    DIV = 269,            /* "/"  */
    ASSIGN = 270,         /* "="  */
    KW_BOOLEAN = 271,     /* "bool"  */
    KW_DO = 272,          /* "do"  */
    KW_ELSE = 273,        /* "else"  */
    KW_FLOAT = 274,       /* "float"  */
    KW_FOR = 275,         /* "for"  */
    KW_IF = 276,          /* "if"  */
    KW_INT = 277,         /* "int"  */
    KW_PRINT = 278,       /* "print"  */
    KW_RETURN = 279,      /* "return"  */
    KW_VOID = 280,        /* "void"  */
    KW_WHILE = 281,       /* "while"  */
    INT_LITERAL = 282,    /* INT_LITERAL  */
    FLOAT_LITERAL = 283,  /* FLOAT_LITERAL  */
    BOOL_LITERAL = 284,   /* BOOL_LITERAL  */
    STRING_LITERAL = 285, /* STRING_LITERAL  */
    IDENT = 286,          /* IDENT  */
};

typedef union {
	char *string;
	double floatValue;
	int intValue;
} YYSTYPE;

extern int yylex(void);
extern int yylineno;
extern FILE *yyin;
extern YYSTYPE yylval;

#endif /* !YY_YY_SRC_LIB_PARSER_TAB_H_INCLUDED  */
