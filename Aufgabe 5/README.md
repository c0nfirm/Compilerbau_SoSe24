# Minako-Interpreter: Syntax und Semantik

Der Minako-Interpreter soll die Syntax und Semantik der Sprache C1 implementieren. Dieses Dokument dokumentiert
die syntaktischen und semantischen Regeln des Projektes. 

## Syntax

Hier ist die durch Minako implementierte C1-Grammatik in [W3C-EBNF](https://www.w3.org/TR/REC-xml/#sec-notation).

```ebnf
program            ::= item*;
item               ::= declassignment ";" | functiondefinition;

functiondefinition ::= type Ident "(" parameterlist? ")" "{" statementlist "}";
parameterlist      ::= type Ident ("," type Ident)*;
functioncall       ::= Ident "(" (assignment ("," assignment)*)? ")";

statementlist      ::= statement*;
block              ::= "{" statementlist "}";

statement          ::= ifstatement
                     | forstatement
                     | dowhilestatement ";"
                     | whilestatement
                     | returnstatement ";"
                     | print ";"
                     | declassignment ";"
                     | statassignment ";"
                     | functioncall ";"
                     | block
                     | ";"
                     ;

ifstatement        ::= "if" "(" assignment ")" statement ("else" statement)?;
forstatement       ::= "for" "(" (statassignment | declassignment) ";" expr ";" statassignment ")" statement;
dowhilestatement   ::= "do" statement "while" "(" assignment ")";
whilestatement     ::= "while" "(" assignment ")" statement;
returnstatement    ::= "return" assignment?;
print              ::= "print" "(" (assignment | StringLiteral) ")";
declassignment     ::= type Ident ("=" assignment)?;

type               ::= "bool" | "float" | "int" | "void";

statassignment     ::= Ident "=" assignment;
assignment         ::= Ident "=" assignment | expr;
expr               ::= simpexpr ("==" simpexpr | "!=" simpexpr | "<=" simpexpr | ">=" simpexpr | "<" simpexpr | ">" simpexpr)?;
simpexpr           ::= term ("+" term | "-" term | "||" term)*;
term               ::= factor ("*" factor | "/" factor | "&&" factor)*;
factor             ::= "-" factor
                     | IntLiteral
                     | FloatLiteral
                     | BoolLiteral
                     | Ident
                     | functioncall
                     | "(" assignment ")"
                     ;

Ident              ::= [a-zA-Z_][a-zA-Z0-9_]*;
StringLiteral      ::= '"' [^"\n]* '"';
IntLiteral         ::= [0-9]+;
BoolLiteral        ::= "true" | "false";
FloatLiteral       ::= ([0-9]* "." [0-9]+ ([eE] [-+]? [0-9]+)?)
                     | ([0-9]+ [eE] [-+]? [0-9]+);
```

## Statische Semantik

Alle C1-Programme erfüllen über die syntaktischen Anforderungen hinaus auch noch die folgenden Regeln der statischen, also zur Übersetzungszeit geprüften, Semantik.

### 1. Sichtbarkeit

Ein *Sichtbarkeitsbereich* ist ein Abschnitt des Quellcodes, der zu folgenden Metasymbolen reduziert wird:

* `block`
* `forstatement`
* `functiondefinition`, wobei der Funktionsname Teil des äußeren *Sichtbarkeitsbereiches* ist

Die Grammatik erlaubt eine Schichtung von *Sichtbarkeitsbereichen*.

Es gelten folgende Regeln:

1. Es gibt eine parameterlose `main()`-Funktion mit dem Rückgabetyp `void`.
2. Bezeichner müssen vor ihrer Benutzung deklariert werden.
3. In jedem *Sichtbarkeitsbereich* darf ein Bezeichner höchstens ein Mal deklariert werden.
4. Bei der Namensauflösung von Bezeichnern werden die *Sichtbarkeitsbereiche* von innen nach außen durchsucht und der erste Treffer ausgewählt.

### 2. Typenkompatibilität

C1 unterstützt die implizite Konvertierung von `int` nach `float`. Ein Typ wird als *kompatibel* zu einem zweiten Typ bezeichnet, wenn er identisch oder in den anderen konvertierbar ist.

Es gelten folgende Regeln:

1. Die Bedingungen für `while`-, `do while`-, `for`- und `if`-Schleifen müssen boolesche Ausdrücke sein.
2. Der Ausdruck in der `print`-Anweisung darf nicht vom Typ `void` sein.
3. Es dürfen keine Variablen und Parameter vom Typ `void` deklariert werden.
4. Funktionsaufrufe müssen parameterkonform sein, das bedeutet:
    * Parameter- und Argumentlisten müssen die gleiche Länge haben,
    * Argumenttypen müssen paarweise *kompatibel* mit den Parametertypen sein.
5. Der Typ eines Rückgabewertes muss *kompatibel* zum Rückgabetyp der umgebenden Funktion sein.
6. Funktionen mit dem Rückgabetyp `void` dürfen keine Ausdrücke zurückgeben, auch keine vom Typ `void`.
7. Zuweisungen dürfen nur an Variablenbezeichner erfolgen.
8. Der Typ der rechten Seite einer Zuweisung muss *kompatibel* zum Typ der linken Seite sein.

### 3. Ausdrücke und Operatoren

1. Der Funktionsaufrufoperator (`()`) darf nur auf Funktionsbezeichner angewandt werden.
2. Funktionsbezeichner sind nur nach der Anwendung des Funktionsaufrufoperators gültige Ausdrücke.
3. Die Operanden eines Vergleichsausdrucks müssen miteinander *kompatibel* und dürfen nicht vom Typ `void` sein.
4. Die Operanden in logischen Operationen müssen vom Typ `bool` sein.
5. Die Operanden in arithmetischen Operationen (Addition, Subtraktion, Multiplikation, Division und Negation) müssen vom Typ `int` oder `float` sein.
6. Der Ergebnistyp einer Zuweisung ist der Typ der Variablen auf der linken Seite.
7. Der Ergebnistyp einer Vergleichs- oder logischen Operation ist `bool`.
8. Der Ergebnistyp einer arithmetischen Operation ist `float`, wenn einer der beteiligten Operanden vom Typ `float` ist, und ansonsten `int`.

## Dynamische Semantik

### Definiertes Verhalten

Die dynamische oder Laufzeitsemantik von C1-Programmen orientiert sich an der Laufzeitsemantik von C-Programmen:

1. Alle globalen Variablen mit Initialisierungsausdruck werden in der Reihenfolge ihrer Deklaration vor dem Start der Programmausführung initialisiert.
2. Die Programmausführung startet am Anfang von `main()`.
3. Die Kontrollstrukturen `if`, `while`, `do while` und `for` haben dieselbe Bedeutung wie in C.
4. Die Liste von Anweisungen in Codeblöcken werden der Reihe nach ausgeführt.
5. Die `print`-Anweisung endet stets mit einem Zeilenende und produziert folgende Ausgaben:
   - Zeichenkettenliterale werden unverändert ausgegeben.
   - Für boolesche Ausdrücke mit dem Wert `1` ist die Ausgabe `"true"`, mit dem Wert `0` ist sie `"false"`.
   - Für integrale Ausdrücke wird der Wert in Dezimaldarstellung ausgegeben.
   - Für gewöhnliche Fließkommaausdrücke wird der Wert in Dezimal- oder wissenschaftlicher Notation ausgegeben, je nachdem welche Darstellung kürzer ist.
   - Für ungewöhnliche Fließkommawerte (NaN und ± unendlich) wird `"nan"` ausgegeben, falls der Wert ungleich sich selbst ist, sowie `"inf"` für den größten und `"-inf"` für den kleinsten Wert.
6. Funktionsrufe, die als Anweisungen auftauchen, also deren Rückgabewert keiner Variablen zugewiesen wird, werden ausgeführt und deren Ergebnis anschließend verworfen.
7. Arithmetische, logische und Vergleichsoperationen haben dieselbe Bedeutung wie in C, werden jedoch in Abweichung zu C stets von links nach rechts berechnet.
8. Die Werte der Ausdrücke in Funktionsargumenten werden von links nach rechts berechnet.
9.  Variablenzuweisungen erlangen direkt im Anschluss der Berechnung des Ausdrucks auf der rechten Seite Gültigkeit.
10. Beim Lesen von Variablen wird der zuletzt geschriebene Wert zurückgegeben.

### Undefiniertes Verhalten

Folgendes Programmverhalten ist undefiniert:

1. Der Überlauf von `int`-Werten durch Addition, Subtraktion, Multiplikation, Division oder Negation ist undefiniert.
2. Das Ergebnis einer Integer-Division durch `0` ist undefiniert.
3. Das Lesen aus uninitialisierten Variablen ist undefiniert.
4. Die Rückkehr aus Funktionen mit Rückgabetyp nicht `void` ohne Nutzung der `return`-Anweisung ist undefiniert, auch wenn der Wert danach verworfen würde.
5. Das Abarbeitungsergebnis rekursiver Funktionen, die während der Ausführung sämtlichen verfügbaren Arbeitsspeicher erschöpfen, ist undefiniert.

Die fünfte Regel soll *nicht* dynamisch getestet werden. Stattdessen ist es zulässig, wenn dieser Fall bei der Ausführung zu undefiniertem Verhalten führt.
