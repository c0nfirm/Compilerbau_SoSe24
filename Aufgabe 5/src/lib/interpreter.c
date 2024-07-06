/***************************************************************************//**
 * @file interpreter.c
 * @brief Enthält den Interpreter für den abstrakten Syntaxbaum eines
 * C1-Programms unter Nutzung der aus der semantischen Analyse berechneten
 * Informationen.
 * 
 * Der Einstiegspunkt für die Interpretation ist die Funktion `interpret`, die
 * den Interpreter und die globalen Variablen initialisiert und die
 * `main`-Funktion des Programms betritt. Das Modul definiert auch die Struktur
 * `Interpreter`, die Funktionen bereitstellt, um Anweisungen und Ausdrücke im
 * AST zu besuchen und auszuwerten, Funktionsaufrufe zu handhaben und
 * Kontrollflusskonstrukte wie Schleifen und Bedingungsanweisungen auszuführen.
 *
 * Zusätzlich enthält die Datei Hilfsfunktionen zum Ausführen von Arithmetik,
 * die im Falle von Überläufen einen Fehler liefern, statt undefiniertes
 * Verhalten zu verursachen.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <math.h>

#include "interpreter.h"

/* Hilfsmakro zum Markieren zu erweiternder Codeabschnitte */
#define TODO(...) do { \
	fprintf(stderr, "noch nicht implementiert: " __VA_ARGS__); \
	putc('\n', stderr); \
	exit(-1); \
} while (0)

/* *** Interne Datentypen *************************************************** */

/**
 * @brief Vereinigung über alle möglichen Werte eines C1-Ausdrucks.
 * 
 * Kann entweder ein boolescher, Integer oder Fließkomma-Wert sein.
 */
typedef union {
	int integral;
	double floating;
	bool boolean;
} Value;

/**
 * @brief Mit dem Initialisierungsstatus markierter Variablenwert.
 */
typedef struct {
	bool init;
	Value val;
} Variable;

/**
 * @brief Laufzeitzustand des Interpreters.
 */
typedef struct {
	const Program *ast;     /**<@brief Zeiger auf den abstrakten Syntaxbaum. */
	const SymDefTable *tab; /**<@brief Zeiger auf die Definitionstabelle. */
	Variable *globals;      /**<@brief Zeiger auf das globale Variablenarray. */
	Variable *locals;       /**<@brief Zeiger auf das lokale Variablenarray. */
	
	/** Rückgabewert und Rückkehrflag. */
	struct {
		Value val;          /**<@brief Rückgabewert. */
		DataType type;      /**<@brief Rückgabetyp. */
		bool returning;     /**<@brief Rückkehrflag. */
	} result;
} Interpreter;

/* *** Interne Hilfsfunktionen ********************************************** */

// Vorwärtsdeklarationen

/** Besucht eine Anweisung. */
static void visitStmt(Interpreter*, const Stmt*);

/** Besucht eine `if`-Anweisung. */
static void visitIfStmt(Interpreter*, const IfStmt*);

/** Besucht eine `for`-Schleife. */
static void visitForStmt(Interpreter*, const ForStmt*);

/** Besucht den Initialisierungteil einer `for`-Schleife. */
static void visitForInit(Interpreter*, const ForInit*);

/** Besucht eine `while`-Schleife. */
static void visitWhileStmt(Interpreter*, const WhileStmt*);

/** Besucht eine `do while`-Schleife. */
static void visitDoWhileStmt(Interpreter*, const WhileStmt*);

/** Besucht eine `return`-Anweisung und setzt den Rückgabewert. */
static void visitReturnStmt(Interpreter*, const Expr*);

/** Besucht eine `print`-Anweisung und schreibt in die Standardausgabe. */
static void visitPrintStmt(Interpreter*, const PrintStmt*);

/** Besucht eine Variablendefinition. */
static void visitVarDef(Interpreter*, const VarDef*);

/** Besucht einen Anweisungsblock und führt jede der Anweisungen aus. */
static void visitBlock(Interpreter*, const Block*);

/** Besucht einen Ausdruck, evaluiert ihn und konvertiert das Ergebnis in einen Zieltyp. */
static Value visitExpr(Interpreter*, const Expr*, DataType);

/** Besucht einen Funktionsruf und konvertiert das Ergebnis in einen Zieltyp. */
static Value visitFuncCall(Interpreter*, const FuncCall*, DataType);

/** Besucht eine binäre Operation und konvertiert das Ergebnis in einen Zieltyp. */
static Value visitBinOpExpr(Interpreter*, const BinOpExpr*, DataType);

/** Besucht eine unäre Minusoperation und konvertiert das Ergebnis in einen Zieltyp. */
static Value visitUnaryMinus(Interpreter*, const Expr*, DataType);

/** Besucht einen Zuweisungausdruck und konvertiert das Ergebnis in einen Zieltyp. */
static Value visitAssign(Interpreter*, const Assign*, DataType);

/** Besucht eine Variable, liest dessen Wert und konvertiert ihn in einen Zieltyp. */
static Value visitLoadVar(Interpreter*, const ResIdent*, DataType);

/** Besucht ein Literal und konvertiert dessen Wert in einen Zieltyp. */
static Value visitLiteral(Interpreter*, const Literal*, DataType);

/* ****** Geprüfte Arithmetik *********************************************** */

/**
 * @brief Addiert zwei Integerwerte, falls es dabei nicht zu einem Überlauf
 * kommen würde und gibt andernfalls einen Fehlercode zurück.
 * 
 * @param l    Linke Seite der Addition.
 * @param r    Rechte Seite der Addition.
 * @param out  Zeiger auf die Ergebnisvariable.
 * @return `true`, falls die Addition erfolgreich war,
 *         `false`, falls es dabei zu einem Überlauf gekommen wäre
 */
static bool checkedAdd(int l, int r, int *out) {
	if (r >= 0 ? l > INT_MAX - r : l < INT_MIN - r) { return false; }
	*out = l + r;
	return true;
}

/**
 * @brief Subtrahiert zwei Integerwerte, falls es dabei nicht zu einem Überlauf
 * kommen würde und gibt andernfalls einen Fehlercode zurück.
 * 
 * @param l    Linke Seite der Subtraktion.
 * @param r    Rechte Seite der Subtraktion.
 * @param out  Zeiger auf die Ergebnisvariable.
 * @return `true`, falls die Subtraktion erfolgreich war,
 *         `false`, falls es dabei zu einem Überlauf gekommen wäre
 */
static bool checkedSub(int l, int r, int *out) {
	if (r >= 0 ? l < INT_MIN + r : l > INT_MAX + r) { return false; }
	*out = l - r;
	return true;
}

/**
 * @brief Multipliziert zwei Integerwerte, falls es dabei nicht zu einem
 * Überlauf kommen würde und gibt andernfalls einen Fehlercode zurück.
 * 
 * @param l    Linke Seite der Multiplikation.
 * @param r    Rechte Seite der Multiplikation.
 * @param out  Zeiger auf die Ergebnisvariable.
 * @return `true`, falls die Multiplikation erfolgreich war,
 *         `false`, falls es dabei zu einem Überlauf gekommen wäre
 */
static bool checkedMul(int l, int r, int *out) {
	int res = (unsigned int) l * (unsigned int) r;
	bool overflow = l != 0 && res / l != r;
	
	if (!overflow) {
		*out = res;
	}
	
	return !overflow;
}

/**
 * @brief Dividiert zwei Integerwerte, falls es dabei nicht zu einem Überlauf
 * kommen würde und gibt andernfalls einen Fehlercode zurück.
 * 
 * @param l    Linke Seite der Division.
 * @param r    Rechte Seite der Division.
 * @param out  Zeiger auf die Ergebnisvariable.
 * @return `true`, falls die Division erfolgreich war,
 *         `false`, falls es dabei zu einem Überlauf gekommen wäre
 */
static bool checkedDiv(int l, int r, int *out) {
	if (r == 0 || (l == INT_MIN && r == -1)) { return false; }
	*out = l / r;
	return true;
}

/**
 * @brief Negiert einen Integerwert, falls es dabei nicht zu einem Überlauf
 * kommen würde und gibt andernfalls einen Fehlercode zurück.
 * 
 * @param l    Zu negierender Wert.
 * @param out  Zeiger auf die Ergebnisvariable.
 * @return `true`, falls die Negation erfolgreich war,
 *         `false`, falls es dabei zu einem Überlauf gekommen wäre
 */
static bool checkedNeg(int l, int *out) {
	if (l == INT_MIN) { return false; }
	*out = -l;
	return true;
}

/* ****** Fehlerbehandlung ************************************************** */

/**
 * @brief Bricht die Ausführung mit einer Fehlermeldung ab.
 * 
 * Diese Funktion soll gerufen werden, bevor während der Interpretation
 * undefiniertes Programmverhalten auftreten würde.
 */
static noreturn void bail(const char *msg, ...) {
	va_list args;
	
	va_start(args, msg);
	fputs("Runtime error: ", stderr);
	vfprintf(stderr, msg, args);
	putc('\n', stderr);
	va_end(args);
	
	exit(3);
}

/**
 * @brief Bricht die Ausführung ab, wenn ein mutmaßlich unerreichbarer
 * Controllpfad ausgeführt werden soll.
 */
#define unreachable() \
	bail("unreachable code in function '%s()', line %u", __func__, __LINE__)

/* ****** Wertekonvertierung und Indizierung ******************************** */

/**
 * @brief Konvertiert einen Wert von einem Quell- in einen Zieldatentyp.
 */
static Value cast(Value base, DataType from, DataType into) {
	if (from == into) { return base; }
	
	if (from == TYPE_INT && into == TYPE_FLOAT) {
		return (Value) { .floating = base.integral };
	}
	
	if (into == TYPE_VOID) { return base; }
	
	bail("cast from `%s` to `%s` is unsupported", TYPE_NAMES[from], TYPE_NAMES[into]);
}

/**
 * @brief Berechnet den spezifischsten Datentyp, der beide Operanden darstellen
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
	
	bail("invalid LUB of `%s` and `%s`", TYPE_NAMES[lhs], TYPE_NAMES[rhs]);
}

/**
 * @brief Löst eine `DefId` in der Definitionstabelle auf und gibt die
 * Definition zurück.
 */
static const DefInfo* indexDef(const Interpreter *self, DefId def_id) {
	if (defIdIsInvalid(def_id))
		bail("unresolved definition id found");
	return &self->tab->definitions[def_id.index];
}

/**
 * @brief Löst eine `ItemId` im abstrakten Syntaxbaum auf und gibt die
 * `Item`-Instanz zurück.
 */
static const Item* indexItem(const Interpreter *self, ItemId item_id) {
	if (itemIdIsInvalid(item_id))
		bail("unresolved item id found");
	return &self->ast->items[item_id.index];
}

/**
 * @brief Liest den Wert einer Variablen.
 * @return Zeiger auf die lokale oder globale Variable.
 */
static Variable* loadVar(const Interpreter *self, const DefInfo *info) {
	switch (info->tag) {
	case SYM_DEF_LOCAL_VAR:
		if (self->locals == NULL)
			bail("attempted to load local variable outside of function");
		return &self->locals[info->var.offset];
		
	case SYM_DEF_GLOBAL_VAR:
		return &self->globals[info->var.offset];
		
	default:
		bail("attempted to read the value of a function");
	}
}

/**
 * @brief Speichert einen Wert in einer Variablen und setzt den
 * Initialisierungsstatus.
 */
static void storeVar(Interpreter *self, const DefInfo *info, Value value) {
	Variable *variable;
	
	switch (info->tag) {
	case SYM_DEF_LOCAL_VAR:
		if (self->locals == NULL)
			bail("attempted to write into a local variable outside of function");
		variable = &self->locals[info->var.offset];
		break;
		
	case SYM_DEF_GLOBAL_VAR:
		variable = &self->globals[info->var.offset];
		break;
		
	default:
		bail("attempted to write a value into a function");
	}
	
	variable->init = true;
	variable->val = value;
}

/* ****** Anweisungen ******************************************************* */

void visitStmt(Interpreter *self, const Stmt *stmt) {
	switch (stmt->tag) {
	case STMT_EMPTY: break;
	case STMT_IF: visitIfStmt(self, &stmt->if_stmt); break;
	case STMT_FOR: visitForStmt(self, &stmt->for_stmt); break;
	case STMT_WHILE: visitWhileStmt(self, &stmt->while_stmt); break;
	case STMT_DO_WHILE: visitDoWhileStmt(self, &stmt->while_stmt); break;
	case STMT_RETURN: visitReturnStmt(self, &stmt->return_stmt); break;
	case STMT_PRINT: visitPrintStmt(self, &stmt->print_stmt); break;
	case STMT_VAR_DEF: visitVarDef(self, &stmt->var_def); break;
	case STMT_ASSIGN: visitAssign(self, &stmt->assign, TYPE_VOID); break;
	case STMT_CALL: visitFuncCall(self, &stmt->call, TYPE_VOID); break;
	case STMT_BLOCK: visitBlock(self, &stmt->block); break;
	}
}

										/*TODO PART*/
void visitIfStmt(Interpreter *self, const IfStmt *if_stmt) {
	printf("visitIfStmt ");
	Value cond = visitExpr(self, &if_stmt->cond, TYPE_BOOL);
	if (cond.boolean){
		visitStmt(self, if_stmt->if_true);
	}
	else if (if_stmt->if_false != NULL){
		visitStmt(self, if_stmt->if_false);
	}
}

void visitForStmt(Interpreter *self, const ForStmt *for_stmt) {
	/**TODO("Initialisierer auswerten, den Körper ausführen und aktualisieren, "
	     "bis die Bedingung zu `false` ausgewertet wird oder die Funktion zurückkehrt");*/
	visitForInit(self, &for_stmt->init);
	visitBlock(self, &for_stmt->block)
	printf("visitForStmt ");
}

void visitForInit(Interpreter *self, const ForInit *for_init) {
	/*TODO("Initialisierungsanweisung auswerten");*/
	switch (for_init->tag){
		case FOR_INIT_VAR_DEF:
			visitVarDef(self, &for_init->var_def);
			break;
		case FOR_INIT_ASSIGN:
			visitAssign(self, &for_init->assign, TYPE_BOOL);
	default:
		break;
	}
}

void visitWhileStmt(Interpreter *self, const WhileStmt *while_stmt) {
	TODO("Körper ausführen, bis die Bedingung zu `false` ausgewertet wird oder "
	     "die Funktion zurückkehren möchte");
}

void visitDoWhileStmt(Interpreter *self, const WhileStmt *while_stmt) {
	TODO("Den Körper mindestens einmal ausführen und dann, bis die Bedingung "
	     "zu `false` ausgewertet wird oder die Funktion zurückkehrt");
}

void visitReturnStmt(Interpreter *self, const Expr *return_stmt) {
	TODO("Das Rückgabeflag und den Rückgabewert gegebenenfalls setzen");
}

void visitPrintStmt(Interpreter *self, const PrintStmt *print_stmt) {
	switch (print_stmt->tag) {
	case PRINT_STRING:
		printf("%s\n", print_stmt->string);
		break;
		
	case PRINT_EXPR: {
		TODO("Den Ausdruck auswerten und ausgeben, dabei auch besondere Float-Werte berücksichtigen");
	}}
}

void visitVarDef(Interpreter *self, const VarDef *var_def) {
	TODO("Die Variable gemäß ihrer Definition initialisieren, falls zutreffend");
	

}

void visitBlock(Interpreter *self, const Block *block) {
	/*TODO("Die enthaltenen Anweisungen besuchen oder frühzeitig zurückkehren, falls angefordert");*/
	vecForEach(self, block->statements) {
		visitStmt(self, statements);
	}
}

/* ****** Ausdrücke ********************************************************* */

Value visitExpr(Interpreter *self, const Expr *expr, DataType target_type) {
	switch (expr->tag) {
	case EXPR_INVALID: bail("attempting to visit invalid expr");
	case EXPR_ASSIGN: return visitAssign(self, &expr->assign, target_type);
	case EXPR_BIN_OP: return visitBinOpExpr(self, &expr->bin_op, target_type);
	case EXPR_UNARY_MINUS: return visitUnaryMinus(self, expr->unary_minus, target_type);
	case EXPR_CALL: return visitFuncCall(self, &expr->call, target_type);
	case EXPR_LITERAL: return visitLiteral(self, &expr->literal, target_type);
	case EXPR_VAR: return visitLoadVar(self, &expr->var, target_type);
	}
	
	unreachable();
}

Value visitFuncCall(Interpreter *self, const FuncCall *func_call, DataType target_type) {
	// TODO: 1. Argumente auswerten.
	// TODO: 2. Platz für lokale Variablen reservieren und Parameter mit 
	//          Argumentwerten initialisieren.
	// TODO: 3. Stack-Frame vorbereiten und später wieder abräumen.
	// TODO: 4. Rückgabewert extrahieren und ggf. konvertieren.
	
	const DefInfo *sym = indexDef(self, func_call->res_ident.res);
	
	if (sym->tag != SYM_DEF_FUNC)
		bail("function call resolves to non-function");
	
	const Item *func = indexItem(self, sym->func.item_id);
	
	/* execute the function statements */
	vecForEach(const Stmt *stmt, func->func_def.statements) {
		visitStmt(self, stmt);
		if (self->result.returning) { break; }
	}
	
	/* return the function result */
	return self->result.val;
}

Value visitBinOpExpr(Interpreter *self, const BinOpExpr *bin_op_expr, DataType target_type) {
	TODO("beide Operanden auswerten und die Operation ausführen, dabei besondere "
	     "Vorsicht bei potenziellen Ganzzahlüberläufen walten lassen");
}

Value visitUnaryMinus(Interpreter *self, const Expr *unary_minus, DataType target_type) {
	TODO("den Operanden auswerten und die Operation ausführen, dabei besondere "
	     "Vorsicht bei Ganzzahlüberläufen walten lassen");
}

Value visitAssign(Interpreter *self, const Assign *assign, DataType target_type) {
	TODO("die rechte Seite auswerten und das Ergebnis in der von der linken "
	     "Seite referenzierten Variable speichern");
}

Value visitLoadVar(Interpreter *self, const ResIdent *ident, DataType target_type) {
	TODO("den Wert der Variablen lesen, dabei darauf achten, keinen "
	     "uninitialisierten Wert zu lesen");
}

Value visitLiteral(Interpreter *self, const Literal *lit, DataType target_type) {
	switch (lit->tag) {
	case LITERAL_INT:
		return cast((Value) { .integral = lit->iVal }, TYPE_INT, target_type);
		
	case LITERAL_FLOAT:
		return cast((Value) { .floating = lit->fVal }, TYPE_FLOAT, target_type);
		
	case LITERAL_BOOL:
		return cast((Value) { .boolean = lit->bVal }, TYPE_BOOL, target_type);
	}
	
	unreachable();
}

/* *** Einstiegspunkt ******************************************************* */

void interpret(const Program *ast, const SymDefTable *tab) {
	Interpreter self = {
		.ast = ast,
		.tab = tab,
		.globals = malloc(sizeof(Variable) * tab->global_count)
	};
	
	if (self.globals == NULL) {
		fputs("out-of-memory error", stderr);
		exit(-1);
	}
	
	/* initialisiere die globalen Variablen in Deklarationsreihenfolge */
	vecForEach(const Item *item, ast->items) {
		if (item->tag == ITEM_GLOBAL_VAR) {
			visitVarDef(&self, &item->var_def);
		}
	}
	
	/* besuche die `main`-Funktion */
	FuncCall main_func = {
		.res_ident = { .ident = "main", .res = tab->main_func }
	};
	visitFuncCall(&self, &main_func, TYPE_VOID);
	free(self.globals);
}
