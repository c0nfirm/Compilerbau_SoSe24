/***************************************************************************//**
 * @file interpreter.h
 * @author Dorian Weber
 * 
 * @brief Schnittstelle des C1-Interpreters.
 * 
 * # Überblick
 * 
 * In dieser Headerdatei wird die Interpretationsfunktion des Interpreters
 * exponiert. Diese kann nach erfolgreicher syntaktischer und semantischer
 * Prüfung mit dem erzeugten abstrakten Syntaxbaum aus der syntaktischen und der
 * Symboltabelle aus der semantischen Analyse ausgeführt werden.
 * 
 * Die Funktion beendet das Programm mit einer Fehlermeldung, falls es
 * andernfalls bei der Ausführung zu undefiniertem Verhalten gekommen wäre.
 ******************************************************************************/

#ifndef INTERPRETER_H_INCLUDED
#define INTERPRETER_H_INCLUDED

#include "ast.h"
#include "symtab.h"

/**
 * @brief Interpretiert den übergebenen, annotierten abstrakten Syntaxbaum unter
 * Nutzung der Symboltabelle.
 * 
 * Die Interpretation wird mit einem Fehler abgebrochen, falls es bei der
 * Ausführung zu undefiniertem Verhalten gekommen wäre.
 * 
 * @param ast  Der annotierte abstrakte Syntaxbaum.
 * @param defs Die Symboltabelle aus der semantischen Analyse.
 */
extern void interpret(const Program *ast, const SymDefTable *defs);

#endif
