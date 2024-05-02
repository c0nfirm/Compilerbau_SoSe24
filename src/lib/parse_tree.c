/***************************************************************************//**
 * @file parse_tree.c
 * Implementation of the parser for the parse tree.
 ******************************************************************************/

#include "parse_tree.h"
#include "vec.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#define TODO(MSG) do { \
	fprintf(stderr, "unvollständig: " MSG "\n"); \
	exit(-1); \
} while (0)

/* ********************************************************* public interface */

ParseResult rootFromStr(Root *root, const char *str) {
	// TODO: Funktionskörper vervollständigen
	ParseResult rc = PARSE_OK;
	Stmt *stmt_list;
	Expr *expr_stack;
	
	vecInit(stmt_list);
	vecInit(expr_stack);
	
	for (char c = *str; c != '\0'; c = *++str) {
		switch (c) {
		case '+':
			vecPush(stmt_list) = c;
			break;
			
		case '-':
			vecPush(stmt_list) = c;
			break;
			
		case '*':
			vecPush(stmt_list) = c;
			break;
			
		case '/':
			vecPush(stmt_list) = c;
			break;
			
		case '=':
			vecPush(stmt_list) = c;
			break;
			
		default:
			if (isspace(c))
				continue;
			
			if (isdigit(c)) {
				int ic = c - 0;
				vecPush(stmt_list) = ic;
				break;
			}
			
			if (islower(c)) {
				vecPush(stmt_list) = c;
				break;
			}
			
			return PARSE_ERR_LEXICAL;
			goto err;
		}
	}
	
	if (!vecIsEmpty(expr_stack)) {
		vecPush(stmt_list) = stmtFromExpr(vecPop(expr_stack));
	}
	
	if (!vecIsEmpty(expr_stack)) {
		return PARSE_ERR_LEXICAL;
		goto err;
	}
	
	root->stmt_list = stmt_list;
	vecRelease(expr_stack);
	return PARSE_OK;
	
err:
	vecForEach(Stmt *stmt, stmt_list) { stmtRelease(stmt); }
	vecRelease(stmt_list);
	
	vecForEach(Expr *expr, expr_stack) { exprRelease(expr); }
	vecRelease(expr_stack);
	
	return rc;
}
Root rootFromStmt(Stmt stmt) {
	Root root;
	vecInit(root.stmt_list);
	vecPush(root.stmt_list) = stmt;
	return root;
}

void rootRelease(Root *root) {
	vecForEach(Stmt *stmt, root->stmt_list) {
		stmtRelease(stmt);
	}
	
	vecRelease(root->stmt_list);
}
void rootPushStmt(Root *root, Stmt stmt) {
	vecPush(root->stmt_list) = stmt;
}

Stmt stmtFromExpr(Expr expr) {
	return (Stmt) {
		.tag = STMT_EXPR,
		.expr = expr
	};
}
Stmt stmtFromSet(char var, Expr val) {
	return (Stmt) {
		.tag = STMT_SET,
		.set = {
			.var = var,
			.expr = val
		}
	};
}
void stmtRelease(Stmt *stmt) {
	switch (stmt->tag) {
	case STMT_EXPR:
		exprRelease(&stmt->expr);
		break;
		
	case STMT_SET:
		exprRelease(&stmt->set.expr);
		break;
	}
}

Expr exprFromInt(int val) {
	return (Expr) {
		.tag = EXPR_INT,
		.val = val
	};
}
Expr exprFromVar(char var) {
	return (Expr) {
		.tag = EXPR_VAR,
		.var = var
	};
}
Expr exprFromAdd(Expr lhs, Expr rhs) {
	Expr result = {
		.tag = EXPR_ADD,
		.op = {
			.lhs = malloc(sizeof(Expr)),
			.rhs = malloc(sizeof(Expr))
		}
	};
	
	*result.op.lhs = lhs;
	*result.op.rhs = rhs;
	
	return result;
}
Expr exprFromAdd(Expr lhs, Expr rhs) {
	Expr result = {
		.tag = EXPR_ADD,
		.op = {
			.lhs = malloc(sizeof(Expr)),
			.rhs = malloc(sizeof(Expr))
		}
	};
	
	*result.op.lhs = lhs;
	*result.op.rhs = rhs;
	
	return result;
}
Expr exprFromSub(Expr lhs, Expr rhs) {
	Expr result = {
		.tag = EXPR_SUB,
		.op = {
			.lhs = malloc(sizeof(Expr)),
			.rhs = malloc(sizeof(Expr))
		}
	};
	
	*result.op.lhs = lhs;
	*result.op.rhs = rhs;
	
	return result;
}
Expr exprFromMul(Expr lhs, Expr rhs) {
	Expr result = {
		.tag = EXPR_MUL,
		.op = {
			.lhs = malloc(sizeof(Expr)),
			.rhs = malloc(sizeof(Expr))
		}
	};
	
	*result.op.lhs = lhs;
	*result.op.rhs = rhs;
	
	return result;
}
Expr exprFromDiv(Expr lhs, Expr rhs) {
	Expr result = {
		.tag = EXPR_DIV,
		.op = {
			.lhs = malloc(sizeof(Expr)),
			.rhs = malloc(sizeof(Expr))
		}
	};
	
	*result.op.lhs = lhs;
	*result.op.rhs = rhs;
	
	return result;
}
void exprRelease(Expr *expr) {
	switch (expr->tag) {
	case EXPR_INT:
	case EXPR_VAR:
		break;
		
	default:
		exprRelease(expr->op.lhs);
		exprRelease(expr->op.rhs);
		free(expr->op.lhs);
		free(expr->op.rhs);
	}
}

/* *************************************************************** unit tests */

#ifdef TEST
bool exprEq(const Expr *lhs, const Expr *rhs) {
	if (lhs->tag != rhs->tag) {
		return false;
	}
	
	switch (lhs->tag) {
	case EXPR_INT:
		return lhs->val == rhs->val;
	case EXPR_VAR:
		return lhs->var == rhs->var;
	default:
		return exprEq(lhs->op.lhs, rhs->op.lhs)
		    && exprEq(lhs->op.rhs, rhs->op.rhs);
	}
}

bool stmtEq(const Stmt *lhs, const Stmt *rhs) {
	if (lhs->tag != rhs->tag) {
		return false;
	}
	
	switch (lhs->tag) {
	case STMT_EXPR:
		return exprEq(&lhs->expr, &rhs->expr);
		
	case STMT_SET:
		return lhs->set.var == rhs->set.var
		     && exprEq(&lhs->set.expr, &rhs->set.expr);
	}
	
	return true;
}

bool rootEq(const Root *lhs, const Root *rhs) {
	if (vecLen(lhs->stmt_list) != vecLen(rhs->stmt_list)) {
		return false;
	}
	
	for (int i = 0; i < vecLen(lhs->stmt_list); ++i) {
		if (!stmtEq(&lhs->stmt_list[i], &rhs->stmt_list[i])) {
			return false;
		}
	}
	
	return true;
}

#define EXPECT(COND) \
	if (!(COND)) { \
		fprintf(stderr, #COND " failed "); \
		return false; \
	}

bool parse_add(void) {
	Root expected = rootFromStmt(stmtFromExpr(
		exprFromAdd(
			exprFromInt(4),
			exprFromInt(2)
		)
	));
	Root tree;
	
	EXPECT(rootFromStr(&tree, "4 2 +") == PARSE_OK);
	EXPECT(rootEq(&tree, &expected));
	
	return true;
}

bool parse_sub(void) {
	Root expected = rootFromStmt(stmtFromExpr(
		exprFromSub(
			exprFromInt(4),
			exprFromInt(2)
		)
	));
	Root tree;
	
	EXPECT(rootFromStr(&tree, "4 2 -") == PARSE_OK);
	EXPECT(rootEq(&tree, &expected));
	
	return true;
}

bool parse_mul(void) {
	Root expected = rootFromStmt(stmtFromExpr(
		exprFromMul(
			exprFromInt(4),
			exprFromInt(2)
		)
	));
	Root tree;
	
	EXPECT(rootFromStr(&tree, "4 2 *") == PARSE_OK);
	EXPECT(rootEq(&tree, &expected));
	
	return true;
}

bool parse_div(void) {
	Root expected = rootFromStmt(stmtFromExpr(
		exprFromDiv(
			exprFromInt(4),
			exprFromInt(2)
		)
	));
	Root tree;
	
	EXPECT(rootFromStr(&tree, "4 2 /") == PARSE_OK);
	EXPECT(rootEq(&tree, &expected));
	
	return true;
}

bool parse_set(void) {
	Root expected = rootFromStmt(stmtFromSet('a', exprFromInt(1)));
	Root tree;
	
	EXPECT(rootFromStr(&tree, "a 1 =") == PARSE_OK);
	EXPECT(rootEq(&tree, &expected));
	
	return true;
}

bool parse_error1(void) {
	Root root;
	EXPECT(rootFromStr(&root, "1 2 ^") != PARSE_OK);
	return true;
}

bool parse_error2(void) {
	Root root;
	EXPECT(rootFromStr(&root, "1 2 3 + ") != PARSE_OK);
	return true;
}

bool parse_error3(void) {
	Root root;
	EXPECT(rootFromStr(&root, "1 2 + *") != PARSE_OK);
	return true;
}

bool parse_error4(void) {
	Root root;
	EXPECT(rootFromStr(&root, "1 1 =") != PARSE_OK);
	return true;
}

#endif
