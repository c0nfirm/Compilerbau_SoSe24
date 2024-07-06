/***************************************************************************//**
 * @file symtab.c
 * @author Dorian Weber
 * @brief Enthält die Umsetzung der Symboltabelle.
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include "symtab.h"

/* *** internal helpers ***************************************************** */

/**
 * Makro zur vereinfachten Implementation von Debug-Print Routinen für
 * Struktur-Typen.
 */
#define STRUCT(TYPE, ...) do {            \
	const char *sep = "";                 \
	fputs("(" #TYPE ") {", out);          \
	indent++;                             \
	__VA_ARGS__;                          \
	indent--;                             \
	fprintf(out, "\n%*s}", indent*4, ""); \
} while (0)

/**
 * Makro zur Debug-Print Ausgabe von Varianten eines Variantentyps.
 */
#define CHOICE(NAME, ...) do { \
	fputs(#NAME "(", out);     \
	__VA_ARGS__;               \
	fputs(")", out);           \
} while (0)

/**
 * Makro zur Ausgabe von Strukturfeldern mit Strukturtyp.
 */
#define FIELD(NAME, ...) do {                                \
	fprintf(out, "%s\n%*s." #NAME " = ", sep, indent*4, ""); \
	__VA_ARGS__;                                             \
	sep = ",";                                               \
} while (0)

/**
 * Makro zur Ausgabe von Strukturfeldern mit Vektortyp.
 */
#define VEC_FIELD(NAME, ...) do {                             \
	fprintf(out, "%s\n%*s." #NAME " = [", sep, indent*4, ""); \
	if (vecLen(self->NAME) == 0) {                            \
		fputs("]", out); break;                               \
	}                                                         \
	sep = "";                                                 \
	indent++;                                                 \
	for (unsigned int i = 0; i < vecLen(self->NAME); ++i) {   \
		fprintf(out, "%s\n%*s[%u] = ", sep, indent*4, "", i); \
		sep = ",";                                            \
		__VA_ARGS__;                                          \
	}                                                         \
	indent--;                                                 \
	sep = ",";                                                \
	fprintf(out, "\n%*s]", indent*4, "");                     \
} while (0)

/**
 * Makro zur Ausgabe von Strukturfeldern mit Zeigertyp.
 */
#define STR_FIELD(NAME) do {                                                   \
	fprintf(out, "%s\n%*s." #NAME " = \"%s\"", sep, indent*4, "", self->NAME); \
	sep = ",";                                                                 \
} while (0)

/**
 * Makro zur Ausgabe von Strukturfeldern mit Integertyp.
 */
#define INT_FIELD(NAME) do {                                               \
	fprintf(out, "%s\n%*s." #NAME " = %i", sep, indent*4, "", self->NAME); \
	sep = ",";                                                             \
} while (0)

/* *** Debug-Print Mechanismus ********************************************** */

/* forward declarations */
static void defInfoPrint(const DefInfo*, const DefInfo*, unsigned int, FILE*);
static void funcInfoPrint(const FuncInfo*, const DefInfo*, unsigned int, FILE*);
static void symVarPrint(const VarInfo*, const DefInfo*, unsigned int, FILE*);
static void symbolPrint(const SymtabSymbol*, const DefInfo*, unsigned int, FILE*);

void defInfoPrint(
	const DefInfo *self,
	const DefInfo *definitions,
	unsigned int indent,
	FILE *out
) {
	switch (self->tag) {
	case SYM_DEF_FUNC:
		CHOICE(Func, {
			fprintf(out, "\"%s\", ", self->ident);
			funcInfoPrint(&self->func, definitions, indent, out);
		});
		break;
		
	case SYM_DEF_GLOBAL_VAR:
		CHOICE(GlobalVar, {
			fprintf(out, "\"%s\", ", self->ident);
			symVarPrint(&self->var, definitions, indent, out);
		});
		break;
		
	case SYM_DEF_LOCAL_VAR:
		CHOICE(LocalVar, {
			fprintf(out, "\"%s\", ", self->ident);
			symVarPrint(&self->var, definitions, indent, out);
		});
		break;
	}
}

void funcInfoPrint(
	const FuncInfo *self,
	const DefInfo *definitions,
	unsigned int indent,
	FILE *out
) {
	STRUCT(FuncInfo, {
		FIELD(item_id, {
			if (itemIdIsInvalid(self->item_id)) {
				fputs("(ItemId) None", out);
			} else {
				fprintf(out, "(ItemId) %u", self->item_id.index);
			}
		});
		FIELD(return_type, astDataTypePrint(&self->return_type, indent, out));
		INT_FIELD(param_count);
		VEC_FIELD(local_vars,
			defInfoPrint(&definitions[self->local_vars[i].index], definitions, indent, out)
		);
	});
}

void symVarPrint(
	const VarInfo *self,
	const DefInfo *definitions,
	unsigned int indent,
	FILE *out
) {
	STRUCT(VarInfo, {
		FIELD(data_type, astDataTypePrint(&self->data_type, indent, out));
		INT_FIELD(offset);
	});
}

void symbolPrint(const SymtabSymbol* self, const DefInfo* definitions, unsigned int indent, FILE* out) {
	STRUCT(SymtabSymbol, {
		STR_FIELD(ident);
		
		if (self->prev_record != -1u) {
			INT_FIELD(prev_record);
		}
		
		FIELD(def, defInfoPrint(&definitions[self->def.index], definitions, indent, out));
	});
}

/* *** Hilfsfunktionen für die Symbotabelle ********************************* */

static void symDefVecRelease(DefInfo *self) {
	vecForEach(DefInfo *def, self) {
		if (def->tag == SYM_DEF_FUNC) {
			vecRelease(def->func.local_vars);
		}
		
		free(def->ident);
	}
	
	vecRelease(self);
}

static int isGlobalScope(const Symtab *self) {
	return vecLen(self->vars_in_scope) == 1;
}

static DefId define(Symtab *self, DefInfo def) {
	unsigned int sym = vecLen(self->decl);
	unsigned int prev = dictGet(&self->map, def.ident);
	DefId def_id = { vecLen(self->definitions) };
	
	/* bail if we have detected a double-declaration within the same scope */
	if (prev != -1u && sym - prev <= vecTop(self->vars_in_scope)) {
		return INVALID_DEF_ID;
	}
	
	/* append this symbol to the current scope */
	vecTop(self->vars_in_scope)++;
	vecPush(self->definitions) = def;
	vecPush(self->decl) = (SymtabSymbol) {
		.def = def_id,
		.ident = def.ident,
		.prev_record = prev
	};
	
	/* update the definition */
	dictInsert(&self->map, def.ident, sym);
	
	return def_id;
}

/**
 * @internal
 * @brief Dupliziert eine Zeichenkette und gibt einen Zeiger auf die Kopie
 * zurück.
 */
static char* stringDup(const char *str) {
	size_t len = strlen(str);
	char *mem = malloc(len + 1);
	
	if (mem == NULL) {
		fputs("out-of-memory error", stderr);
		exit(-1);
	}
	
	memcpy(mem, str, len + 1);
	return mem;
}

/* *** implementation ******************************************************* */

/* ****** Symbol Table ****************************************************** */

Symtab symtabNew(void) {
	Symtab result;
	
	/* initialize all of the dynamic containers */
	dictInit(&result.map);
	vecInit(result.definitions);
	vecInit(result.decl);
	vecInit(result.vars_in_scope);
	vecPush(result.vars_in_scope) = 0;
	result.global_count = 0;
	result.current_func = INVALID_DEF_ID;
	
	return result;
}

void symtabRelease(Symtab *self) {
	symDefVecRelease(self->definitions);
	dictRelease(&self->map);
	vecRelease(self->decl);
	vecRelease(self->vars_in_scope);
}

bool symtabDefineFunc(Symtab *self, const char *ident, DataType return_type) {
	assert(isGlobalScope(self));
	
	DefInfo def = {
		.tag = SYM_DEF_FUNC,
		.ident = stringDup(ident),
		.func = {
			.item_id = INVALID_ITEM_ID,
			.return_type = return_type
		}
	};
	
	self->current_func = define(self, def);
	if (defIdIsInvalid(self->current_func)) {
		vecRelease(def.func.local_vars);
		free(def.ident);
		return false;
	}
	
	return true;
}

bool symtabDefineParam(Symtab *self, const char *ident, DataType data_type) {
	assert(!isGlobalScope(self));
	if (defIdIsInvalid(symtabDefineVar(self, ident, data_type)))
		return false;
	
	self->definitions[self->current_func.index].func.param_count++;
	return true;
}

DefId symtabDefineVar(Symtab *self, const char *ident, DataType data_type) {
	/* set the kind of variable and calculate the offset into the stack */
	unsigned int offset;
	int tag;
	
	if (isGlobalScope(self)) {
		offset = self->global_count;
		tag = SYM_DEF_GLOBAL_VAR;
	} else {
		offset = vecLen(symtabCurrentFunc(self)->local_vars);
		tag = SYM_DEF_LOCAL_VAR;
	}
	
	/* prepare the variable definition */
	DefInfo def = {
		.tag = tag,
		.ident = stringDup(ident),
		.var = { .data_type = data_type, .offset = offset }
	};
	
	/* define the variable and add it to the list of local function variables */
	DefId def_id = define(self, def);
	
	/* short-circuit in case of double declaration */
	if (defIdIsInvalid(def_id)) {
		free(def.ident);
		return def_id;
	}
	
	if (isGlobalScope(self)) {
		++self->global_count;
	} else {
		/* add the definition to the local variables */
		FuncInfo *func = &symtabIndex(self, self->current_func)->func;
		vecPush(func->local_vars) = def_id;
	}
	
	/* return the definition ID */
	return def_id;
}

void symtabScopeEnter(Symtab *self) {
	vecPush(self->vars_in_scope) = 0;
}

void symtabScopeLeave(Symtab *self) {
	assert(!isGlobalScope(self));
	
	unsigned int count = vecPop(self->vars_in_scope);
	for (unsigned int i = 0; i < count; ++i) {
		SymtabSymbol sym = vecPop(self->decl);
		
		if (sym.prev_record == -1u) {
			dictRemove(&self->map, sym.ident);
		} else {
			dictInsert(&self->map, sym.ident, sym.prev_record);
		}
	}
}

DefId symtabResolve(const Symtab *self, const char *ident) {
	unsigned int sym = dictGet(&self->map, ident);
	if (sym == -1u) { return INVALID_DEF_ID; }
	return self->decl[sym].def;
}

DefInfo* symtabIndex(const Symtab *self, DefId def_id) {
	return (defIdIsInvalid(def_id)) ? NULL : &self->definitions[def_id.index];
}

FuncInfo* symtabCurrentFunc(const Symtab *self) {
	DefInfo *func = symtabIndex(self, self->current_func);
	if (isGlobalScope(self)) return NULL;
	assert(func->tag == SYM_DEF_FUNC);
	return &func->func;
}

void symtabPrint(const Symtab *self, unsigned int indent, FILE *out) {
	/* pretend that the definitions looks like a Vec of Vec instead of a single
	 * segmented Vec; this helps in seeing the structure of the scopes */
	STRUCT(Symtab, {
		unsigned int k = 0;
		
		VEC_FIELD(vars_in_scope, {
			if (self->vars_in_scope[i] == 0) {
				fputs("[]", out);
				continue;
			}
			
			sep = "";
			fputs("[", out);
			++indent;
			for (unsigned int j = 0; j < self->vars_in_scope[i]; ++j, ++k, sep = ",") {
				fprintf(out, "%s\n%*s[%u] = ", sep, indent*4, "", j);
				symbolPrint(&self->decl[k], self->definitions, indent, out);
			}
			--indent;
			fprintf(out, "\n%*s]", indent*4, "");
		});
	});
	
	putc('\n', out);
}

/* ****** Symbol Definition Table ******************************************* */

SymDefTable symDefTableNew(Symtab *tab, const Program *ast) {
	for (unsigned int i = 0; i < vecLen(ast->items); ++i) {
		const Item *item = &ast->items[i];
		
		/* check if the item is a global variable */
		if (item->tag == ITEM_GLOBAL_VAR) { continue; }
		
		/* fill out ItemIds to the AST for global functions */
		DefInfo *def = symtabIndex(tab, symtabResolve(tab, item->func_def.ident));
		
		/* skip non-resolved function definitions */
		if (def == NULL) { continue; }
		assert(def->tag == SYM_DEF_FUNC);
		
		def->func.item_id.index = i;
	}
	
	/* initialize the SymDefTable */
	SymDefTable result = {
		.definitions = tab->definitions,
		.global_count = tab->global_count,
		.main_func = symtabResolve(tab, "main")
	};
	
	/* release the symbol definitions */
	dictRelease(&tab->map);
	vecRelease(tab->decl);
	vecRelease(tab->vars_in_scope);
	
	return result;
}

void symDefTableRelease(SymDefTable *self) {
	symDefVecRelease(self->definitions);
}

const DefInfo* symDefTableResolve(const SymDefTable *self, DefId def_id) {
	assert(!defIdIsInvalid(def_id));
	return &self->definitions[def_id.index];
}

void symDefTablePrint(const SymDefTable *self, unsigned int indent, FILE *out) {
	STRUCT(SymDefTable, {
		FIELD(main_func, {
			if (defIdIsInvalid(self->main_func)) {
				fputs("(DefId) None", out);
			} else {
				fprintf(out, "(DefId) %u", self->main_func.index);
			}
		});
		INT_FIELD(global_count);
		VEC_FIELD(definitions, defInfoPrint(&self->definitions[i], self->definitions, indent, out));
	});
	
	putc('\n', out);
}
