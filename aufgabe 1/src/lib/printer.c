/***************************************************************************//**
 * @file printer.c
 * Implementation of the printer.
 ******************************************************************************/

#include "printer.h"
#include "vec.h"

/* ******************************************************* private interface */
static void visitExpr(Printer *self, const Expr *expr) {
	// DONE ?
	switch (expr->tag) {
	case EXPR_INT:
		visitExpr(self, expr->op.lhs);
		visitExpr(self, expr->op.rhs);
		break;
		
	case EXPR_VAR:
		visitExpr(self, expr->op.lhs);
		visitExpr(self, expr->op.rhs);
		break;
		
	case EXPR_ADD:
		visitExpr(self, expr->op.lhs);
		visitExpr(self, expr->op.rhs);
		break;
		
	case EXPR_SUB:
		visitExpr(self, expr->op.lhs);
		visitExpr(self, expr->op.rhs);
		break;
		
	case EXPR_MUL:
		visitExpr(self, expr->op.lhs);
		visitExpr(self, expr->op.rhs);
		break;
		
	case EXPR_DIV:
		visitExpr(self, expr->op.lhs);
		visitExpr(self, expr->op.rhs);
		break;
	}
}
static void visitStmt(Printer *self, const Stmt *stmt) {
	// TODO: eventuell Ergänzungen vornehmen
	switch (stmt->tag) {
	case STMT_EXPR:
		visitExpr(self, &stmt->expr);
		break;
		
	case STMT_SET:
		visitExpr(self, &stmt->set.expr);
		break;
	}
}
static void visitRoot(Printer *self, const Root *root) {
	// TODO: eventuell Ergänzungen vornehmen
	vecForEach(const Stmt *stmt, root->stmt_list) {
		visitStmt(self, stmt);
	}
}

/* ******************************************************** public interface */
void printerFormat(Printer *self, const Root *root) {
	visitRoot(self, root);
}

/* *************************************************************** unit tests */
#ifdef TEST
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static bool test(const Root *root, const char *expected) {
	bool rc = true;
	Printer printer = { .out = tmpfile() };
	long len = strlen(expected);
	
	printerFormat(&printer, root);
	
	long act_len = ftell(printer.out);
	if (act_len > len) {
		len = act_len;
	}
	
	char buf[len+1];
	rewind(printer.out);
	fread(buf, 1, len, printer.out);
	buf[len] = '\0';
	
	// trim whitespace from the back of the string to be a bit more permissive
	while (isspace(buf[--len]));
	buf[++len] = '\0';
	
	if (!(rc = (strcmp(buf, expected) == 0))) {
		fprintf(
			stderr, "unexpected output, expected: '%s', actual: '%s'",
			expected, buf
		);
	}
	
	fclose(printer.out);
	return rc;
}

bool printer_add(void) {
	Root root = rootFromStmt(stmtFromExpr(
		exprFromAdd(
			exprFromInt(4),
			exprFromInt(2)
		)
	));
	return test(&root, "(4+2)");
}

bool printer_sub(void) {
	Root root = rootFromStmt(stmtFromExpr(
		exprFromSub(
			exprFromInt(4),
			exprFromInt(2)
		)
	));
	return test(&root, "(4-2)");
}

bool printer_mul(void) {
	Root root = rootFromStmt(stmtFromExpr(
		exprFromMul(
			exprFromInt(4),
			exprFromInt(2)
		)
	));
	return test(&root, "(4*2)");
}

bool printer_div(void) {
	Root root = rootFromStmt(stmtFromExpr(
		exprFromDiv(
			exprFromInt(4),
			exprFromInt(2)
		)
	));
	return test(&root, "(4/2)");
}

bool printer_set(void) {
	Root root = rootFromStmt(stmtFromSet('a', exprFromInt(1)));
	return test(&root, "a=1");
}

#endif
