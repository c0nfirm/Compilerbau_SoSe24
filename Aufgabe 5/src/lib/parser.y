%define parse.error verbose
%define parse.trace
%parse-param {ParseResult *out}

%code requires {
	#include <stdio.h>
	#include <stdarg.h>
	#include "ast.h"
	#include "vec.h"
	#include "symtab.h"
	
	/* Vorwärtsdeklaration für die yyparse()-Funktion */
	typedef struct ParseResult ParseResult;
	
	extern int yylex(void);
	extern int yylineno;
	extern FILE *yyin;
}

%code provides {
	/**
	 * @brief Variantentyp des Ergebnisses des Parsevorganges.
	 */
	struct ParseResult {
		enum ParseResultTag {
			PARSE_OK,
			PARSE_ERR_SYNTAX,
			PARSE_ERR_SEMANTIC
		} tag;
		
		union {
			struct {
				Program ok;
				Symtab tab;
			};
			char err[256];
		};
	};
	
	/**
	 * Die obere Parse-Funktion: wandelt den Eingabestrom in einen AST (`Program`)
	 * um oder gibt eine Fehlermeldung zurück, falls das Programm inkorrekt ist.
	 */
	extern ParseResult astParse(FILE *input);
	
	/**
	 * Speichert die Fehlermeldung für den Rufer.
	 * Die Funktion akzeptiert eine variable Argumentliste und nutzt die Syntax von
	 * printf.
	 * @param out  Zeiger auf das Ausgabeargument der parse-Funktion
	 * @param msg  die Fehlermeldung
	 * @param ...  variable Argumentliste für die Formatierung von \p msg
	 */
	extern void yyerror(ParseResult *out, const char *msg, ...);
}

%code {
	/* Eine externe Variable, die das Hauptprogramm setzt, um die semantischen
	 * Tests zu beeinflussen. Für den Wert 0 werden keine semantischen Tests
	 * durchgeführt, für Werte ungleich 0 werden sie durchgeführt.
	 * 
	 * Das ist notwendig, damit der Referenzinterpreter auch in Anwesenheit
	 * semantischer Fehler einen abstrakten Syntaxbaum ausgeben kann. */
	extern const int SEMANTIC_CHECK;
	
	/**
	 * Meldet einen semantischen Fehler, falls die Bedingung zutrifft.
	 */
	#define DENY(COND, ...) do { \
		if (SEMANTIC_CHECK && (COND)) { \
			yyerror(out, __VA_ARGS__); \
			out->tag = PARSE_ERR_SEMANTIC; \
			YYABORT; \
		} \
	} while (0)
	
	/**
	 * Berechnet den spezifischsten Datentyp, der beide Operanden darstellen
	 * kann.
	 * 
	 * Diese Funktion ist symmetrisch, es wird also davon ausgegangen, dass beide
	 * Seiten der Operation sich konvertieren lassen. Hier wird die implizite
	 * Umwandlung von `int` nach `float` abgebildet.
	 * 
	 * @param lhs  linker Operand
	 * @param rhs  rechter Operand
	 * @return spezifischster Datentyp, der beide Operanden umfasst oder
	 * `TYPE_VOID`, falls es keinen gibt
	 */
	static DataType leastUpperBound(DataType lhs, DataType rhs) {
		if (lhs == rhs) { return lhs; }
		if ((lhs == TYPE_INT && rhs == TYPE_FLOAT)
		||  (lhs == TYPE_FLOAT && rhs == TYPE_INT)) {
			return TYPE_FLOAT;
		}
		
		return TYPE_VOID;
	}
	
	/**
	 * Prüft, ob die rechte Seite einer Zuweisung mit der linken kompatibel ist.
	 * 
	 * Diese Funktion ist asymmetrisch, es wird also davon ausgegangen, dass nur
	 * die rechte Seite sich umwandeln lässt.
	 * 
	 * @param lhs  linker Operand
	 * @param rhs  rechter Operand
	 * @return `1`, falls die Zuweisung der rechten an die linke Seite möglich
	 * ist, `0` ansonsten
	 */
	static inline int compatible(DataType lhs, DataType rhs) {
		return lhs == rhs || (lhs == TYPE_FLOAT && rhs == TYPE_INT);
	}
	
	/**
	 * Gibt den Namen eines Datentyps zurück.
	 */
	static inline const char* typeName(DataType type) {
		return TYPE_NAMES[type];
	}
}

%union {
	/* lexical token types */
	char *string;
	double floatValue;
	int intValue;
	
	/* ast types */
	Item item;
	
	FuncDef func_def;
	FuncParam func_param;
	FuncCall func_call;
	
	Block block;
	
	Stmt stmt;
	IfStmt if_stmt;
	ForStmt for_stmt;
	WhileStmt while_stmt;
	PrintStmt print_stmt;
	VarDef var_def;
	
	Assign assign;
	Expr expr;
	
	DataType type;
	
	/* vectors of ast types */
	FuncParam *func_params;
	Stmt *stmts;
	Expr *exprs;
}

/* define the printer routines for improved debug output */
%printer { fprintf(yyoutput, "\"%s\"", $$); }     <string>
%printer { fprintf(yyoutput, "%g", $$); }         <floatValue>
%printer { fprintf(yyoutput, "%i", $$); }         <intValue>
%printer { astItemPrint(&$$, 0, yyoutput); }      <item>
%printer { astFuncDefPrint(&$$, 0, yyoutput); }   <func_def>
%printer { astFuncParamPrint(&$$, 0, yyoutput); } <func_param>
%printer { astFuncCallPrint(&$$, 0, yyoutput); }  <func_call>
%printer { astBlockPrint(&$$, 0, yyoutput); }     <block>
%printer { astStmtPrint(&$$, 0, yyoutput); }      <stmt>
%printer { astIfStmtPrint(&$$, 0, yyoutput); }    <if_stmt>
%printer { astForStmtPrint(&$$, 0, yyoutput); }   <for_stmt>
%printer { astWhileStmtPrint(&$$, 0, yyoutput); } <while_stmt>
%printer { astPrintStmtPrint(&$$, 0, yyoutput); } <print_stmt>
%printer { astVarDefPrint(&$$, 0, yyoutput); }    <var_def>
%printer { astAssignPrint(&$$, 0, yyoutput); }    <assign>
%printer { astExprPrint(&$$, 0, yyoutput); }      <expr>

/* define destructors in order to prevent memory leaks */
%destructor { free($$); }                 <string>
%destructor { astItemRelease(&$$); }      <item>
%destructor { astFuncDefRelease(&$$); }   <func_def>
%destructor { astFuncParamRelease(&$$); } <func_param>
%destructor { astFuncCallRelease(&$$); }  <func_call>
%destructor { astBlockRelease(&$$); }     <block>
%destructor { astStmtRelease(&$$); }      <stmt>
%destructor { astIfStmtRelease(&$$); }    <if_stmt>
%destructor { astForStmtRelease(&$$); }   <for_stmt>
%destructor { astWhileStmtRelease(&$$); } <while_stmt>
%destructor { astPrintStmtRelease(&$$); } <print_stmt>
%destructor { astVarDefRelease(&$$); }    <var_def>
%destructor { astAssignRelease(&$$); }    <assign>
%destructor { astExprRelease(&$$); }      <expr>

%destructor {
	if ($$ != NULL) {
		vecForEach(FuncParam *e, $$) {
			astFuncParamRelease(e);
		}
		vecRelease($$);
	}
} <func_params>

%destructor {
	if ($$ != NULL) {
		vecForEach(Stmt *e, $$) {
			astStmtRelease(e);
		}
		vecRelease($$);
	}
} <stmts>

%destructor {
	if ($$ != NULL) {
		vecForEach(Expr *e, $$) {
			astExprRelease(e);
		}
		vecRelease($$);
	}
} <exprs>

/* extra token declaration to support versions of bison older than 3.6;
 * ignore the warning in your editor */
%token YYUNDEF

/* used tokens (KW is abbreviation for keyword) */
%token LOG_AND      "&&"
%token LOG_OR       "||"
%token EQ           "=="
%token NEQ          "!="
%token LT           "<"
%token GT           ">"
%token LEQ          "<="
%token GEQ          ">="
%token ADD          "+"
%token SUB          "-"
%token MUL          "*"
%token DIV          "/"
%token ASSIGN       "="
%token KW_BOOLEAN   "bool"
%token KW_DO        "do"
%token KW_ELSE      "else"
%token KW_FLOAT     "float"
%token KW_FOR       "for"
%token KW_IF        "if"
%token KW_INT       "int"
%token KW_PRINT     "print"
%token KW_RETURN    "return"
%token KW_VOID      "void"
%token KW_WHILE     "while"
%token <intValue>   INT_LITERAL
%token <floatValue> FLOAT_LITERAL
%token <intValue>   BOOL_LITERAL
%token <string>     STRING_LITERAL
%token <string>     IDENT

/* workaround for handling dangling else */
/* LOWER_THAN_ELSE stands for a non-existing else */
%nonassoc LOWER_THAN_ELSE
%nonassoc KW_ELSE

%type <item>        item
%type <func_def>    functiondefinition
%type <func_param>  parameter
%type <func_params> parameterlist opt_parameterlist
%type <func_call>   functioncall
%type <block>       block
%type <stmt>        statement returnstatement opt_else
%type <stmts>       statementlist
%type <if_stmt>     ifstatement
%type <for_stmt>    forstatement
%type <while_stmt>  whilestatement dowhilestatement
%type <print_stmt>  print
%type <var_def>     declassignment
%type <assign>      statassignment
%type <expr>        assignment expr simpexpr term factor
%type <exprs>       argumentlist opt_argumentlist

%type <type> type

%%

/* see EBNF grammar for further information */
program:
	itemlist {
		const DefInfo *def = symtabIndex(&out->tab, symtabResolve(&out->tab, "main"));
		
		// nach Regel #1.1
		DENY(def == NULL, "void main() has to exist");
		DENY(def->tag != SYM_DEF_FUNC, "main() must be a function");
		DENY(def->func.return_type != TYPE_VOID, "main() cannot have a return value");
		DENY(def->func.param_count > 0, "main() cannot have parameters");
	}
	;

itemlist:
	/* empty */
	| itemlist item { vecPush(out->ok.items) = $item; }
	;

item:
	declassignment ';' {
		$$ = astItemFromVarDef($declassignment);
	}
	| functiondefinition {
		$$ = astItemFromFuncDef($functiondefinition);
	}
	;

functiondefinition:
	type IDENT[ident] '(' opt_parameterlist[params] ')' {
		bool def = symtabDefineFunc(&out->tab, $ident, $type);
		
		// Regel #1.3
		DENY(!def, "double declaration of symbol '%s' in the file scope", $ident);
		
		symtabScopeEnter(&out->tab);
		
		vecForEach(FuncParam *param, $params) {
			def = symtabDefineParam(&out->tab, param->ident, param->data_type);
			DENY(!def, "double declaration of parameter '%s' in function scope", param->ident);
		}
	} '{'
		statementlist[body]
	'}' {
		$$ = astFuncDefNew($type, $ident, $params, $body);
		symtabScopeLeave(&out->tab);
	}
	;

opt_parameterlist:
	/* empty */ { $$ = NULL; }
	| parameterlist
	;

parameterlist:
	parameter[param] {
		vecInit($$);
		vecPush($$) = $param;
	}
	| parameterlist[list] ',' parameter[param] {
		vecPush($list) = $param;
		$$ = $list;
	}
	;

parameter:
	type IDENT[ident] {
		// Regel #2.3
		DENY(
			$type == TYPE_VOID,
			"parameter '%s' cannot be 'void'",
			$ident
		);
		
		$$ = astFuncParamNew($type, $ident);
	}
	;

functioncall:
	IDENT[ident] '(' opt_argumentlist[args] ')' {
		DefId func = symtabResolve(&out->tab, $ident);
		const DefInfo *def = symtabIndex(&out->tab, func);
		
		// Regel #1.2
		DENY(def == NULL, "undeclared symbol '%s'", $ident);
		
		// Regel #3.1
		DENY(def->tag != SYM_DEF_FUNC, "'%s' cannot be called", $ident);
		
		unsigned int arg_count = vecLen($args);
		
		// Regel #2.4
		DENY(
			def->func.param_count > arg_count,
			"more arguments expected in call to '%s()'", $ident
		);
		DENY(
			def->func.param_count < arg_count,
			"too many arguments in call to '%s()'", $ident
		);
		
		for (unsigned int i = 0; i < arg_count; ++i) {
			const DefInfo *param = symtabIndex(&out->tab, def->func.local_vars[i]);
			DataType expected = param->var.data_type;
			
			// Regel #2.4
			DENY(
				!compatible(expected, $args[i].data_type),
				"type '%s' of argument %u is not compatible with parameter of type '%s' in call to '%s()'",
				typeName($args[i].data_type), i+1, typeName(expected), $ident
			);
		}
		
		$$ = astFuncCallNew($ident, $args);
		$$.res_ident.res = func;
	}
	;

opt_argumentlist:
	/* empty */ { $$ = NULL; }
	| argumentlist
	;

argumentlist:
	assignment[expr] {
		vecInit($$);
		vecPush($$) = $expr;
	}
	| argumentlist[list] ',' assignment[expr] {
		vecPush($list) = $expr;
		$$ = $list;
	}
	;

statementlist:
	/* empty */ {
		vecInit($$);
	}
	| statementlist[list] statement[stmt] {
		vecPush($list) = $stmt;
		$$ = $list;
	}
	;

block:
	'{' {
		symtabScopeEnter(&out->tab);
	} statementlist[stmts] '}' {
		symtabScopeLeave(&out->tab);
		$$ = astBlockNew($stmts);
	}
	;

statement:
	  ifstatement {
		$$ = astStmtFromIfStmt($ifstatement);
	  }
	| forstatement {
		$$ = astStmtFromForStmt($forstatement);
	}
	| whilestatement {
		$$ = astStmtFromWhileStmt($whilestatement);
	}
	| dowhilestatement ';' {
		$$ = astStmtFromDoWhileStmt($dowhilestatement);
	}
	| returnstatement ';'
	| print ';' {
		$$ = astStmtFromPrintStmt($print);
	}
	| declassignment ';' {
		$$ = astStmtFromVarDef($declassignment);
	}
	| statassignment ';' {
		$$ = astStmtFromAssign($statassignment);
	}
	| functioncall ';' {
		$$ = astStmtFromFuncCall($functioncall);
	}
	| block {
		$$ = astStmtFromBlock($block);
	}
	| ';' {
		$$ = astStmtNew();
	}
	;

ifstatement:
	KW_IF '(' assignment[cond] ')' statement[true] opt_else[false] {
		// Regel #2.1
		DENY(
			$cond.data_type != TYPE_BOOL,
			"condition needs to be boolean"
		);
		
		$$ = astIfStmtNew($cond, $true, $false);
	}
	;

/* KW_ELSE hat höhere Präzedenz, so dass die zweite Regel ausgeführt wird,
 * falls 'else' als nächstes in der Eingabe steht */
opt_else:
	/* empty */ %prec LOWER_THAN_ELSE {
		$$ = astStmtNew();
	}
	| KW_ELSE statement[body] {
		$$ = $body;
	}
	;

forstatement:
	KW_FOR '(' {
		symtabScopeEnter(&out->tab);
	} declassignment[init] ';' expr[cond] ';' statassignment[update] ')' statement[body] {
		// Regel #2.1
		DENY(
			$cond.data_type != TYPE_BOOL,
			"condition needs to be boolean"
		);
		
		$$ = astForStmtNew(
			astForInitFromVarDef($init),
			$cond, $update, $body
		);
		symtabScopeLeave(&out->tab);
	}
	| KW_FOR '(' {
		symtabScopeEnter(&out->tab);
	} statassignment[init] ';' expr[cond] ';' statassignment[update] ')' statement[body] {
		// Regel #2.1
		DENY(
			$cond.data_type != TYPE_BOOL,
			"condition needs to be boolean"
		);
		
		$$ = astForStmtNew(
			astForInitFromAssign($init),
			$cond, $update, $body
		);
		symtabScopeLeave(&out->tab);
	}
	;

dowhilestatement:
	KW_DO statement[body] KW_WHILE '(' assignment[cond] ')' {
		// Regel #2.1
		DENY(
			$cond.data_type != TYPE_BOOL,
			"condition needs to be boolean"
		);
		
		$$ = astWhileStmtNew($cond, $body);
	}
	;

whilestatement:
	KW_WHILE '(' assignment[cond] ')' statement[body] {
		// Regel #2.1
		DENY(
			$cond.data_type != TYPE_BOOL,
			"condition needs to be boolean"
		);
		
		$$ = astWhileStmtNew($cond, $body);
	}
	;

returnstatement:
	KW_RETURN {
		const FuncInfo *func = symtabCurrentFunc(&out->tab);
		
		// Regel #2.5
		DENY(
			func->return_type != TYPE_VOID,
			"no return value in function with return type '%s'",
			typeName(func->return_type)
		);
		
		$$ = astStmtFromReturn(NULL);
	}
	| KW_RETURN assignment[expr] {
		const FuncInfo *func = symtabCurrentFunc(&out->tab);
		
		// Regel #2.6
		DENY(
			func->return_type == TYPE_VOID,
			"cannot return expressions from function with return type 'void'"
		);
		
		// Regel #2.5
		DENY(
			!compatible(func->return_type, $expr.data_type),
			"cannot return '%s' from function with return type '%s'",
			typeName($expr.data_type), typeName(func->return_type)
		);
		
		$$ = astStmtFromReturn(&$expr);
	}
	;

print:
	KW_PRINT '(' assignment[expr] ')' {
		// Regel #2.2
		DENY(
			$expr.data_type == TYPE_VOID,
			"cannot print expressions of type 'void'"
		);
		$$ = astPrintStmtFromExpr($expr);
	}
	| KW_PRINT '(' STRING_LITERAL[str] ')' {
		$$ = astPrintStmtFromString($str);
	}
	;

declassignment:
	type IDENT[ident] {
		DefId def_id = symtabDefineVar(&out->tab, $ident, $type);
		
		// Regel #2.3
		DENY(
			$type == TYPE_VOID,
			"variable '%s' of type 'void'", $ident
		);
		
		// Regel #1.3
		DENY(
			defIdIsInvalid(def_id),
			"double declaration of symbol '%s'", $ident
		);
		
		$$ = astVarDefNew($type, $ident, NULL);
		$$.res_ident.res = def_id;
	}
	| type IDENT[ident] ASSIGN assignment[init] {
		DefId def_id = symtabDefineVar(&out->tab, $ident, $type);
		
		// Regel #2.3
		DENY(
			$type == TYPE_VOID,
			"variable '%s' of type 'void'", $ident
		);
		
		// Regel #1.3
		DENY(
			defIdIsInvalid(def_id),
			"double declaration of symbol '%s'", $ident
		);
		
		// Regel #2.8
		DENY(
			!compatible($type, $init.data_type),
			"variable of type '%s' cannot be assigned value of type '%s'",
			typeName($type), typeName($init.data_type)
		);
		
		$$ = astVarDefNew($type, $ident, &$init);
		$$.res_ident.res = def_id;
	}
	;

type:
	KW_BOOLEAN { $$ = TYPE_BOOL; }
	| KW_FLOAT { $$ = TYPE_FLOAT; }
	| KW_INT   { $$ = TYPE_INT; }
	| KW_VOID  { $$ = TYPE_VOID; }
	;

statassignment:
	IDENT[lhs] ASSIGN assignment[rhs] {
		DefId def_id = symtabResolve(&out->tab, $lhs);
		const DefInfo *def = symtabIndex(&out->tab, def_id);
		
		// Regel #1.2
		DENY(def == NULL, "undeclared symbol '%s'", $lhs);
		
		// Regel #2.7
		DENY(def->tag == SYM_DEF_FUNC, "cannot assign value to function '%s'", $lhs);
		
		// Regel #2.8
		DENY(
			!compatible(def->var.data_type, $rhs.data_type),
			"variable of type '%s' cannot be assigned value of type '%s'",
			typeName(def->var.data_type), typeName($rhs.data_type)
		);
		
		$$ = astAssignNew($lhs, $rhs);
		$$.lhs.res = def_id;
	}
	;

assignment:
	IDENT[lhs] ASSIGN assignment[rhs] {
		DefId def_id = symtabResolve(&out->tab, $lhs);
		const DefInfo *def = symtabIndex(&out->tab, def_id);
		
		// Regel #1.2
		DENY(def == NULL, "undeclared symbol '%s'", $lhs);
		
		// Regel #2.7
		DENY(def->tag == SYM_DEF_FUNC, "cannot assign value to function '%s'", $lhs);
		
		// Regel #2.8
		DENY(
			!compatible(def->var.data_type, $rhs.data_type),
			"variable of type '%s' cannot be assigned value of type '%s'",
			typeName(def->var.data_type), typeName($rhs.data_type)
		);
		
		$$ = astExprFromAssign(astAssignNew($lhs, $rhs));
		$$.assign.lhs.res = def_id;
		
		// Regel ##3.6
		$$.data_type = def->var.data_type;
	}
	| expr
	;

expr:
	simpexpr
	| simpexpr[lhs] EQ  simpexpr[rhs] {
		DataType lub = leastUpperBound($lhs.data_type, $rhs.data_type);
		// Regel #3.3
		DENY(
			lub == TYPE_VOID,
			"expression of type '%s' cannot be tested for equality with expression of type '%s'",
			typeName($lhs.data_type), typeName($rhs.data_type)
		);
		
		$$ = astExprFromBinOpExpr($lhs, $rhs, BIN_OP_EQ);
		
		// Regel #3.7
		$$.data_type = TYPE_BOOL;
	}
	| simpexpr[lhs] NEQ simpexpr[rhs] {
		DataType lub = leastUpperBound($lhs.data_type, $rhs.data_type);
		// Regel #3.3
		DENY(
			lub == TYPE_VOID,
			"expression of type '%s' cannot be tested for equality with expression of type '%s'",
			typeName($lhs.data_type), typeName($rhs.data_type)
		);
		
		$$ = astExprFromBinOpExpr($lhs, $rhs, BIN_OP_NEQ);
		
		// Regel #3.7
		$$.data_type = TYPE_BOOL;
	}
	| simpexpr[lhs] LEQ simpexpr[rhs] {
		DataType lub = leastUpperBound($lhs.data_type, $rhs.data_type);
		// Regel #3.3
		DENY(
			lub == TYPE_VOID,
			"expression of type '%s' cannot be compared with expression of type '%s'",
			typeName($lhs.data_type), typeName($rhs.data_type)
		);
		
		$$ = astExprFromBinOpExpr($lhs, $rhs, BIN_OP_LEQ);
		
		// Regel #3.7
		$$.data_type = TYPE_BOOL;
	}
	| simpexpr[lhs] GEQ simpexpr[rhs] {
		DataType lub = leastUpperBound($lhs.data_type, $rhs.data_type);
		// Regel #3.3
		DENY(
			lub == TYPE_VOID,
			"expression of type '%s' cannot be compared with expression of type '%s'",
			typeName($lhs.data_type), typeName($rhs.data_type)
		);
		
		$$ = astExprFromBinOpExpr($lhs, $rhs, BIN_OP_GEQ);
		
		// Regel #3.7
		$$.data_type = TYPE_BOOL;
	}
	| simpexpr[lhs] LT simpexpr[rhs] {
		DataType lub = leastUpperBound($lhs.data_type, $rhs.data_type);
		// Regel #3.3
		DENY(
			lub == TYPE_VOID,
			"expression of type '%s' cannot be compared with expression of type '%s'",
			typeName($lhs.data_type), typeName($rhs.data_type)
		);
		
		$$ = astExprFromBinOpExpr($lhs, $rhs, BIN_OP_LT);
		
		// Regel #3.7
		$$.data_type = TYPE_BOOL;
	}
	| simpexpr[lhs] GT simpexpr[rhs] {
		DataType lub = leastUpperBound($lhs.data_type, $rhs.data_type);
		// Regel #3.3
		DENY(
			lub == TYPE_VOID,
			"expression of type '%s' cannot be compared with expression of type '%s'",
			typeName($lhs.data_type), typeName($rhs.data_type)
		);
		
		$$ = astExprFromBinOpExpr($lhs, $rhs, BIN_OP_GT);
		
		// Regel #3.7
		$$.data_type = TYPE_BOOL;
	}
	;

simpexpr:
	term
	| simpexpr[lhs] ADD term[rhs] {
		DataType lub = leastUpperBound($lhs.data_type, $rhs.data_type);
		
		// Regel #3.5
		DENY(
			lub != TYPE_INT && lub != TYPE_FLOAT,
			"cannot add '%s' and '%s'",
			typeName($lhs.data_type), typeName($rhs.data_type)
		);
		
		$$ = astExprFromBinOpExpr($lhs, $rhs, BIN_OP_ADD);
		
		// Regel #3.8
		$$.data_type = lub;
	}
	| simpexpr[lhs] SUB term[rhs] {
		DataType lub = leastUpperBound($lhs.data_type, $rhs.data_type);
		
		// Regel #3.5
		DENY(
			lub != TYPE_INT && lub != TYPE_FLOAT,
			"cannot subtract '%s' and '%s'",
			typeName($lhs.data_type), typeName($rhs.data_type)
		);
		
		$$ = astExprFromBinOpExpr($lhs, $rhs, BIN_OP_SUB);
		
		// Regel #3.8
		$$.data_type = lub;
	}
	| simpexpr[lhs] LOG_OR term[rhs] {
		// Regel #3.4
		DENY(
			$lhs.data_type != TYPE_BOOL || $rhs.data_type != TYPE_BOOL,
			"'bool' operands expected for logic operation"
		);
		
		$$ = astExprFromBinOpExpr($lhs, $rhs, BIN_OP_LOG_OR);
		
		// Regel #3.7
		$$.data_type = TYPE_BOOL;
	}
	;

term:
	factor
	| term[lhs] MUL factor[rhs] {
		DataType lub = leastUpperBound($lhs.data_type, $rhs.data_type);
		// Regel #3.5
		DENY(
			lub != TYPE_INT && lub != TYPE_FLOAT,
			"cannot multiply '%s' and '%s'",
			typeName($lhs.data_type), typeName($rhs.data_type)
		);
		
		$$ = astExprFromBinOpExpr($lhs, $rhs, BIN_OP_MUL);
		
		// Regel #3.8
		$$.data_type = lub;
	}
	| term[lhs] DIV factor[rhs] {
		DataType lub = leastUpperBound($lhs.data_type, $rhs.data_type);
		// Regel #3.5
		DENY(
			lub != TYPE_INT && lub != TYPE_FLOAT,
			"cannot divide '%s' and '%s'",
			typeName($lhs.data_type), typeName($rhs.data_type)
		);
		
		$$ = astExprFromBinOpExpr($lhs, $rhs, BIN_OP_DIV);
		
		// Regel #3.8
		$$.data_type = lub;
	}
	| term[lhs] LOG_AND factor[rhs] {
		// Regel #3.4
		DENY(
			$lhs.data_type != TYPE_BOOL || $rhs.data_type != TYPE_BOOL,
			"'bool' operands expected for logic operation"
		);
		
		$$ = astExprFromBinOpExpr($lhs, $rhs, BIN_OP_LOG_AND);
		
		// Regel #3.7
		$$.data_type = TYPE_BOOL;
	}
	;

factor:
	SUB factor[val] {
		// Regel #3.5
		DENY(
			$val.data_type != TYPE_INT && $val.data_type != TYPE_FLOAT,
			"cannot apply unary minus to values of type '%s'",
			typeName($val.data_type)
		);
		$$ = astExprFromUnaryMinus($val);
		$$.data_type = $val.data_type;
	}
	| INT_LITERAL[lit] {
		$$ = astExprFromLiteral(astLiteralFromInt($lit));
		$$.data_type = TYPE_INT;
	}
	| FLOAT_LITERAL[lit] {
		$$ = astExprFromLiteral(astLiteralFromFloat($lit));
		$$.data_type = TYPE_FLOAT;
	}
	| BOOL_LITERAL[lit] {
		$$ = astExprFromLiteral(astLiteralFromBool($lit));
		$$.data_type = TYPE_BOOL;
	}
	| IDENT[id] {
		DefId def_id = symtabResolve(&out->tab, $id);
		const DefInfo *def = symtabIndex(&out->tab, def_id);
		
		// Regel #1.2
		DENY(def == NULL, "undeclared symbol '%s'", $id);
		
		// Regel #3.2
		DENY(
			def->tag == SYM_DEF_FUNC,
			"cannot read value of function '%s'", $id
		);
		
		$$ = astExprFromIdent($id);
		$$.var.res = def_id;
		
		if (SEMANTIC_CHECK)
			$$.data_type = def->var.data_type;
	}
	| functioncall {
		const DefInfo *def = symtabIndex(
			&out->tab, symtabResolve(&out->tab, $functioncall.res_ident.ident)
		);
		$$ = astExprFromFuncCall($functioncall);
		
		if (SEMANTIC_CHECK)
			$$.data_type = def->func.return_type;
	}
	| '(' assignment ')' {
		$$ = $assignment;
	}
	;

%%

void yyerror(ParseResult *out, const char* msg, ...) {
	va_list args;
	int len = 0;
	
	/* free the space for the ok-branch and set the error code */
	if (out->tag == PARSE_OK) {
		symtabRelease(&out->tab);
		astProgramRelease(&out->ok);
	}
	
	out->tag = PARSE_ERR_SYNTAX;
	
	/* print the message into the buffer */
	va_start(args, msg);
	len = snprintf(out->err, sizeof(out->err), "Error in line %d: ", yylineno);
	vsnprintf(out->err + len, sizeof(out->err) - len, msg, args);
	va_end(args);
}

ParseResult astParse(FILE *input) {
	ParseResult out = {
		.tag = PARSE_OK,
		.ok = astProgramNew(),
		.tab = symtabNew()
	};
	yyin = input;
	yyparse(&out);
	return out;
}
