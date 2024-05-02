/***************************************************************************//**
 * @file calculator.c
 * Implementation of the calculator.
 ******************************************************************************/

#include "calculator.h"
#include "vec.h"

/* ******************************************************* private interface */

static void visitExpr(Calculator *self, const Expr *expr) {
	// TODO: eventuell Ergänzungen vornehmen
	switch (expr->tag) {
	case EXPR_INT:
		break;

	case EXPR_VAR:
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

static void visitStmt(Calculator *self, const Stmt *stmt) {
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

static void visitRoot(Calculator *self, const Root *root) {
	// TODO: eventuell Ergänzungen vornehmen
	vecForEach(const Stmt *stmt, root->stmt_list) {
		visitStmt(self, stmt);
	}
}

/* ******************************************************** public interface */

int calculatorCalc(Calculator *self, Root *root) {
	// TODO: eventuell Ergänzungen vornehmen
	visitRoot(self, root);
	return 0;
}

/* *************************************************************** unit tests */

#ifdef TEST
#include <stdio.h>

#define EXPECT(COND) \
	if (!(COND)) { \
		fprintf(stderr, #COND " failed "); \
		return false; \
	}

bool calc_add(void) {
	Calculator calc;
	Root root = rootFromStmt(stmtFromExpr(
		exprFromAdd(
			exprFromInt(4),
			exprFromInt(2)
		)
	));
	
	EXPECT(calculatorCalc(&calc, &root) == 6);
	return true;
}

bool calc_sub(void) {
	Calculator calc;
	Root root = rootFromStmt(stmtFromExpr(
		exprFromSub(
			exprFromInt(4),
			exprFromInt(2)
		)
	));
	
	EXPECT(calculatorCalc(&calc, &root) == 2);
	return true;
}

bool calc_mul(void) {
	Calculator calc;
	Root root = rootFromStmt(stmtFromExpr(
		exprFromMul(
			exprFromInt(4),
			exprFromInt(2)
		)
	));
	
	EXPECT(calculatorCalc(&calc, &root) == 8);
	return true;
}

bool calc_div(void) {
	Calculator calc;
	Root root = rootFromStmt(stmtFromExpr(
		exprFromDiv(
			exprFromInt(4),
			exprFromInt(2)
		)
	));
	
	EXPECT(calculatorCalc(&calc, &root) == 2);
	return true;
}

bool calc_set(void) {
	Calculator calc;
	Root root = rootFromStmt(stmtFromSet('a', exprFromInt(1)));
	
	EXPECT(calculatorCalc(&calc, &root) == 0);
	return true;
}

bool calc_vars(void) {
	Root root = rootFromStmt(stmtFromSet('i', exprFromInt(1)));
	
	rootPushStmt(&root, stmtFromSet('j', exprFromInt(2)));
	rootPushStmt(&root, stmtFromExpr(
		exprFromAdd(exprFromVar('i'), exprFromVar('j'))
	));
	
	Calculator calc;
	EXPECT(calculatorCalc(&calc, &root) == 3);
	return true;
}

#endif
