/***************************************************************************//**
 * @file symtab.h
 * @author Dorian Weber
 * 
 * @brief Schnittstelle und Strukturen der Symboltabelle für unseren
 * C1-Interpreter.
 * 
 * # Überblick
 * 
 * Diese Headerdatei definiert die Datenstrukturen und Funktionen zur Verwaltung
 * einer Symboltabelle, die für die semantische Analyse im Parser nützlich ist.
 * Die Symboltabelle speichert Informationen über Bezeichner, ihre Datentypen
 * und schichtet ihre Sichtbarkeitsbereiche.
 * 
 * Die wichtigsten Strukturen umfassen:
 * - `FuncInfo`: Repräsentiert eine Funktion inklusive Rückgabetyp und lokalen
 *   Variablen.
 * - `VarInfo`: Repräsentiert eine Variable inklusive ihres Datentyps und Index
 *   in der Definitionstabelle.
 * - `DefInfo`: Übergeordnete Struktur für eine Definition, die sowohl Funktionen
 *   als auch Variablen umfassen kann.
 * - `SymtabSymbol`: Repräsentiert ein Symbol in der Symboltabelle.
 * - `Symtab`: Die Hauptstruktur der Symboltabelle, die Bezeichner und ihre
 *   Definitionen verwaltet.
 * 
 * Die bereitgestellten Funktionen ermöglichen das Erzeugen und Freigeben von
 * Symboltabellen, das Definieren von Funktionen und Variablen, das Betreten und
 * Verlassen von Sichtbarkeitsbereichen sowie das Suchen und Auflösen von
 * Bezeichnern.
 ******************************************************************************/

#ifndef SYMTAB_H_INCLUDED
#define SYMTAB_H_INCLUDED

/* *** Includes ************************************************************* */

#include <stdio.h>
#include <stdbool.h>
#include "ast.h"
#include "dict.h"
#include "vec.h"

/* *** Strukturen *********************************************************** */

/**
 * @brief Struktur für semantische Informationen über Funktionen.
 */
typedef struct FuncInfo {
	ItemId item_id;           /**<@brief Index der `Item`-Liste in `Program`. */
	DataType return_type;     /**<@brief Rückgabetyp der Funktion. */
	DefId *local_vars;        /**<@brief Lokale Variablen der Funktion. */
	unsigned int param_count; /**<@brief Parameteranzahl des Stack-Frames. */
} FuncInfo;

/**
 * @brief Struktur für semantische Informationen über Variablen.
 */
typedef struct VarInfo {
	DataType data_type;   /**<@brief Datentyp der Variablen. */
	unsigned int offset;  /**<@brief Index der Variablen. */
} VarInfo;

/**
 * @brief Variantentyp für semantische Informationen.
 */
typedef struct DefInfo {
	/**
	 * @brief Tag zur Unterscheidung zwischen Funktions-, lokalen und globalen
	 * Variablendefinitionen.
	 */
	enum {
		SYM_DEF_FUNC,      /**<@brief Funktionsdefinition. */
		SYM_DEF_LOCAL_VAR, /**<@brief Lokale Variablendefinition. */
		SYM_DEF_GLOBAL_VAR /**<@brief Globale Variablendefinition. */
	} tag;
	
	char *ident; /**<@brief Bezeichner des Elementes. */
	
	union {
		FuncInfo func; /**<@brief Funktionsspezifische Informationen. */
		VarInfo var;   /**<@brief Variablenspezifische Informationen. */
	};
} DefInfo;

/**
 * @brief Struktur mit semantischen Informationen, die in Kombination mit dem
 * abstrakten Syntaxbaum für die Interpretation und weitere Verarbeitung
 * nützlich ist.
 * 
 * Beinhaltet eine Tabelle mit Symboldefinitionen, die Anzahl globaler Variablen
 * und die `DefId` für den Programmeinstiegspunkt des C1-Programms.
 */
typedef struct SymDefTable {
	DefId main_func;
	unsigned int global_count;
	DefInfo *definitions;
} SymDefTable;

/**
 * @brief Struktur eines Symbols in der Symboltabelle.
 */
typedef struct SymtabSymbol {
	const char *ident;        /**<@brief Bezeichner im Quellcode. */
	unsigned int prev_record; /**<@brief Eintrag der Vorgängerdefinition. */
	DefId def;                /**<@brief Index in die Definitionstabelle. */
} SymtabSymbol;

/**
 * @brief Struktur der Symboltabelle.
 */
typedef struct {
	/** Wörterbuch für Bezeichner auf Eintragsindex in den `decl`-Vektor. */
	Dict map;
	
	/** Anzahl der globalen Variablen. */
	unsigned int global_count;
	
	/** Stack aller derzeit deklarierten Symbole. */
	SymtabSymbol *decl;
	
	/** Stack der Anzahl lokaler Symbole pro Sichtbarkeitsbereich. */
	unsigned int *vars_in_scope;
	
	/** `DefId` der aktuell geparsten Funktion. */
	DefId current_func;
	
	/** Vektor aller Symboldefinitionen, der durch `DefId` indiziert wird. */
	DefInfo *definitions;
} Symtab;

/* *** Öffentliche Schnittstelle ******************************************** */

/* ****** Symbol Table ****************************************************** */

/**
 * @brief Erzeugt eine neue Symboltabelle.
 * @return Eine neue Symboltabelle.
 */
extern Symtab symtabNew(void);

/**
 * @brief Gibt den Speicher der Symboltabelle frei.
 * @param self Die Symboltabelle.
 */
extern void symtabRelease(Symtab *self);

/**
 * @brief Definiert eine neue Funktion in der Symboltabelle.
 * 
 * Die Funktionsdefinition scheitert, falls die Funktion bereits existiert.
 * 
 * @param self        Die Symboltabelle.
 * @param ident       Der Bezeichner der Funktion.
 * @param return_type Der Rückgabetyp der Funktion.
 * @return `true`, wenn die Funktionsdefinition erfolgreich war, sonst `false`.
 */
extern bool symtabDefineFunc(Symtab *self, const char *ident, DataType return_type);

/**
 * @brief Definiert einen neuen Parameter einer Funktion in der Symboltabelle.
 * 
 * Die Parameterdefinition scheitert, falls der Parameter bereits existiert.
 * 
 * @param self      Die Symboltabelle.
 * @param ident     Der Bezeichner des Parameters.
 * @param data_type Der Datentyp des Parameters.
 * @return `true`, wenn die Parameterdefinition erfolgreich war, sonst `false`.
 */
extern bool symtabDefineParam(Symtab *self, const char *ident, DataType data_type);

/**
 * @brief Definiert eine neue Variable in der Symboltabelle.
 * 
 * Die Variablendefinition scheitert, falls die Variable bereits im aktuellen
 * Sichtbarkeitsbereich existiert.
 * 
 * @param self      Die Symboltabelle.
 * @param ident     Der Bezeichner der Variablen.
 * @param data_type Der Datentyp der Variablen.
 * @return Die `DefId` der Variablen, wenn die Definition erfolgreich war,
 * sonst `INVALID_DEF_ID`.
 */
extern DefId symtabDefineVar(Symtab *self, const char *ident, DataType data_type);

/**
 * @brief Betritt einen neuen Sichtbarkeitsbereich.
 * @param self Die Symboltabelle.
 */
extern void symtabScopeEnter(Symtab *self);

/**
 * @brief Verlässt den aktuellen Sichtbarkeitsbereich.
 * @param self Die Symboltabelle.
 */
extern void symtabScopeLeave(Symtab *self);

/**
 * @brief Sucht einen Bezeichner in der Symboltabelle.
 * 
 * @param self  Die Symboltabelle.
 * @param ident Der Bezeichner, der gesucht wird.
 * @return Die Definition des Bezeichners oder `INVALID_DEF_ID`, wenn der Bezeichner
 * nicht gefunden wurde.
 */
extern DefId symtabResolve(const Symtab *self, const char *ident);

/**
 * @brief Löst eine Definition anhand ihrer ID auf.
 * 
 * @param self   Die Symboltabelle.
 * @param def_id Die ID der Definition.
 * @return Ein Zeiger auf die Definition oder `NULL` falls `INVALID_DEF_ID`
 * übergeben wurde.
 */
extern DefInfo* symtabIndex(const Symtab *self, DefId def_id);

/**
 * @brief Gibt die aktuell definierte Funktion zurück.
 * 
 * @param self Die Symboltabelle.
 * @return Ein Zeiger auf die aktuelle Funktionsdefinition oder `NULL`,
 * im globalen Sichtbarkeitsbereich.
 */
extern FuncInfo* symtabCurrentFunc(const Symtab *self);

/**
 * @brief Gibt den aktuellen Zustand der blockstrukturierten Symboltabelle in
 * einen Ausgabestrom aus.
 * 
 * Diese Funktion kann beim Debugging der semantischen Analyse helfen, da sie
 * zeigt, welche Symbole für welchen Sichtbarkeitsbereich wie definiert sind.
 * 
 * @param self   Die Symboltabelle.
 * @param indent Die Einrückung nach der ersten Zeile.
 * @param out    Der Ausgabestrom.
 */
extern void symtabPrint(const Symtab *self, unsigned int indent, FILE *out);

/* ****** Symbol Definition Table ******************************************* */

/**
 * @brief Wandelt die Symboltabelle in eine Definitionstabelle um.
 * 
 * Die Ressourcen der Symboltabelle sind danach freigegeben. Nur die
 * Definitionstabelle überlebt den Ruf.
 * 
 * @param tab Die Symboltabelle.
 * @param ast Ein konstanter Zeiger auf die Wurzel des abstrakten Syntaxbaumes.
 * @return Die Definitionstabelle.
 */
extern SymDefTable symDefTableNew(Symtab *tab, const Program *ast);

/**
 * @brief Gibt den Speicher der Definitionstabelle frei.
 * @param self Die Definitionstabelle.
 */
extern void symDefTableRelease(SymDefTable *self);

/**
 * @brief Gibt die Definitionstabelle in einen Ausgabestrom aus.
 * 
 * @param self   Die Definitionstabelle.
 * @param indent Die Einrückung nach der ersten Zeile.
 * @param out    Der Ausgabestream.
 */
extern void symDefTablePrint(const SymDefTable *self, unsigned int indent, FILE *out);

#endif
