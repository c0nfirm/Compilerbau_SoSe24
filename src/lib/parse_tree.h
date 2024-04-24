/***************************************************************************//**
 * @file parse_tree.h
 *
 * This header provides functionality to parse and evaluate simple mathematical
 * expressions and assignments. It includes a parser that constructs a parse
 * tree from strings in [reverse polish notation], which can then be evaluated
 * or manipulated.
 *
 * ## Examples
 *
 * Basic parsing and evaluation:
 * ```
 * Root root;
 * const char *expr = "1 2 + 3 4 * 5 6 / -";
 *
 * switch (rootFromStr(&root, expr)) {
 * case PARSE_OK:
 * 	// traverse of evaluate `root` here
 * default:
 * 	printf("Error parsing expression\n");
 * }
 * ```
 *
 * [reverse polish notation]: https://en.wikipedia.org/wiki/Reverse_Polish_notation
 ******************************************************************************/

#ifndef PARSE_TREE_H_INCLUDED
#define PARSE_TREE_H_INCLUDED

/* *** structures *********************************************************** */

/**
 * Represents all possible expressions that can be parsed and evaluated.
 *
 * This [tagged union] covers basic integer constants, variables, and binary
 * operations (addition, subtraction, multiplication, division).
 * 
 * [tagged union]: https://en.wikipedia.org/wiki/Tagged_union
 */
typedef struct Expr {
	enum {
		EXPR_INT, ///< Integer constant.
		EXPR_VAR, ///< Variable, identified by a single character.
		EXPR_ADD, ///< Addition operation between two expressions.
		EXPR_SUB, ///< Subtraction operation between two expressions.
		EXPR_MUL, ///< Multiplication operation between two expressions.
		EXPR_DIV  ///< Division operation between two expressions.
	} tag; ///< Tag to identify the type of expression.

	union {
		int val;  ///< Holds the integer value for EXPR_INT.
		char var; ///< Holds the letter identifying the variable for EXPR_VAR.
		struct {
			struct Expr *lhs; ///< Left-hand side expression.
			struct Expr *rhs; ///< Right-hand side expression.
		} op; ///< Structure holding operands for binary operations.
	};
} Expr;

/**
 * Defines the different types of statements that can appear in the parse tree.
 *
 * Each statement can be either an expression or an assignment of a value to a
 * variable.
 */
typedef struct Stmt {
	enum {
		STMT_EXPR, ///< Represents a statement that is purely an expression.
		STMT_SET   ///< Represents the assignment of an expression to a variable.
	} tag; ///< Tag to identify the type of statement.

	union {
		Expr expr; ///< Expression for STMT_EXPR.
		struct {
			char var;  ///< Variable to which a result is assigned for STMT_SET.
			Expr expr; ///< Expression to be assigned for STMT_SET.
		} set; ///< Structure holding a variable assignment statement.
	};
} Stmt;

/**
 * Represents the root of a parse tree.
 *
 * This struct holds a list of statements parsed from a string representation
 * of expressions and assignments.
 */
typedef struct Root {
	/**
	 * Pointer to a vector of Stmt structures representing the statements in
	 * the parse tree.
	 */
	Stmt *stmt_list;
} Root;

/**
 * Defines error types that can occur during the parsing of expressions.
 *
 * Errors are categorized into lexical, syntax, and semantic types depending
 * on the nature of the error.
 */
typedef enum ParseResult {
	/**Indicates a successful parse run. */
	PARSE_OK,
	/**A lexical error, due to invalid character input. */
	PARSE_ERR_LEXICAL,
	/**A syntax error, due to improper arrangement of tokens. */
	PARSE_ERR_SYNTAX,
	/**A semantic error, due to assignment to a non-variable expression. */
	PARSE_ERR_SEMANTIC
} ParseResult;

/* *** public interface ***************************************************** */

/**
 * Parses a string into a Root struct, constructing a list of statements.
 *
 * This method uses a simple stack-based parser to convert strings in
 * [reverse polish notation] into statements and expressions.
 *
 * Returns an error if the parsing fails due to invalid input.
 *
 * [reverse polish notation]: https://en.wikipedia.org/wiki/Reverse_Polish_notation
 */
extern ParseResult rootFromStr(Root *root, const char *str);

/**
 * Constructs a Root instance from a single statement.
 * @param stmt statement to be used as the single element of the list
 * @return the initialized tree
 */
extern Root rootFromStmt(Stmt stmt);

/**
 * Frees all memory associated with a Root structure, including its statements.
 * @param self Root structure to be freed
 */
extern void rootRelease(Root *self);

/**
 * Adds a statement to the Root's statement list.
 * @param self Root structure to which the statement will be added
 * @param stmt the statement to add.
 */
extern void rootPushStmt(Root *self, Stmt stmt);

/**
 * Initializes a Stmt structure as an expression statement.
 * @param expr the expression to use in initializing the statement
 * @return the initialized statement
 */
extern Stmt stmtFromExpr(Expr expr);

/**
 * Initializes a Stmt structure as an assignment statement.
 * @param var  the variable to which the expression's value will be assigned
 * @param val  the expression whose result will be assigned to the variable
 * @return the initialized statement
 */
extern Stmt stmtFromSet(char var, Expr val);

/**
 * Frees any resources associated with a Stmt structure.
 * @param stmt Stmt structure to be freed
 */
extern void stmtRelease(Stmt *stmt);

/**
 * Initializes an Expr structure as an integer constant.
 * @param val  the integer value to store in the expression
 * @return the initialized expression
 */
extern Expr exprFromInt(int val);

/**
 * Initializes an Expr structure as a variable.
 * @param var  the variable character to store in the expression
 * @return the initialized expression
 */
extern Expr exprFromVar(char var);

/**
 * Initializes an Expr structure as an addition operation.
 * @param lhs  the left-hand side expression of the addition
 * @param rhs  the right-hand side expression of the addition
 * @return the initialized expression
 */
extern Expr exprFromAdd(Expr lhs, Expr rhs);

/**
 * Initializes an Expr structure as a subtraction operation.
 * @param lhs  the left-hand side expression of the subtraction
 * @param rhs  the right-hand side expression of the subtraction
 * @return the initialized expression
 */
extern Expr exprFromSub(Expr lhs, Expr rhs);

/**
 * Initializes an Expr structure as a multiplication operation.
 * @param lhs  the left-hand side expression of the multiplication
 * @param rhs  the right-hand side expression of the multiplication
 * @return the initialized expression
 */
extern Expr exprFromMul(Expr lhs, Expr rhs);

/**
 * Initializes an Expr structure as a division operation.
 * @param lhs  the left-hand side expression of the division
 * @param rhs  the right-hand side expression of the division
 * @return the initialized expression
 */
extern Expr exprFromDiv(Expr lhs, Expr rhs);

/**
 * Frees any resources associated with an Expr structure.
 * @param expr Expr structure to be freed
 */
extern void exprRelease(Expr *expr);

/* *** unit tests *********************************************************** */

#ifdef TEST
#include <stdbool.h>

#define PARSE_TREE_TESTS \
	X(parse_add)         \
	X(parse_sub)         \
	X(parse_mul)         \
	X(parse_div)         \
	X(parse_set)         \
	X(parse_error1)      \
	X(parse_error2)      \
	X(parse_error3)      \
	X(parse_error4)

#define X(TEST) extern bool TEST(void);
PARSE_TREE_TESTS
#undef X

/**
 * Deep-compares two tree roots for equality.
 * @return `true` if they are equal, `false` if they differ
 */
extern bool rootEq(const Root *lhs, const Root *rhs);

/**
 * Deep-compares two tree statements for equality.
 * @return `true` if they are equal, `false` if they differ
 */
extern bool stmtEq(const Stmt *lhs, const Stmt *rhs);

/**
 * Deep-compares two tree expressions for equality.
 * @return `true` if they are equal, `false` if they differ
 */
extern bool exprEq(const Expr *lhs, const Expr *rhs);

#endif /* TEST */
#endif /* PARSE_TREE_H_INCLUDED */
