/***************************************************************************//**
 * @file vec.h
 * @author Dorian Weber
 * @brief Generischer Vektortyp.
 * 
 * @details
 * Diese Datei enthält eine Implementierung eines generischen Vektortyps. Um zu
 * bestimmen, welche Art von Elementen dieser Vektor enthält, deklariert man
 * eine Variable, die ein Zeiger auf den Typ der Elemente ist. Dieser Zeiger
 * kann dann initialisiert und verwendet werden, als ob er auf ein Array
 * variabler Länge zeigt.
 * 
 * Hier ist ein Anwendungsbeispiel:
 * @code
 * int *vec = NULL;
 * 
 * vecPush(vec) = 1;
 * vecPush(vec) = 2;
 * vecPush(vec) = 3;
 * 
 * while (!vecIsEmpty(vec))
 *     printf("%i\n", vecPop(vec));
 * 
 * vecRelease(vec);
 * @endcode
 * 
 * Viele der verwendeten Funktionen sind als Makros implementiert, die die
 * Variable aktualisieren, die den Zeiger auf das Array hält, obwohl es scheint,
 * als ob während des gesamten Programms nur ein Wert verwendet wird. Dies macht
 * es riskant, Zeiger auf das Array in verschiedenen Variablen zu halten, da
 * Größenänderungen diese ungültig machen können. Obwohl diese Probleme subtil
 * und potenziell fehleranfällig sind — dies ist eine extrem undichte
 * Abstraktion — habe ich festgestellt, dass es dennoch einen enormen
 * Produktivitätsschub darstellt; man muss sich nur vage darüber im Klaren sein,
 * was unter der Haube vor sich geht, um die geeignete Teilmenge von
 * Programmiertechniken zu verwenden, die korrekt funktionieren.
 ******************************************************************************/

#ifndef VEC_H_INCLUDED
#define VEC_H_INCLUDED

/* *** includes ************************************************************* */

#include <stddef.h>

/* *** structures *********************************************************** */

/**
 * @brief Struktur, die dem Array zur Buchhaltung vorangestellt ist.
 */
typedef struct {
    size_t len; /**< @brief Anzahl der Elemente im Vektor. */
    size_t cap; /**< @brief Kapazität des reservierten Speichers. */
} VecHeader;

/* *** interface ************************************************************ */

/**
 * @internal
 * Reserviert Speicher für das Array.
 * 
 * Die Länge des Arrays wird auf 0 initialisiert.
 * 
 * @param size      Die Größe der Elemente im Array
 * @param capacity  Die anfängliche Mindestkapazität
 * 
 * @return Der Zeiger auf ein neues Array
 */
extern void* vecInit(size_t capacity, size_t size);

/**
 * @brief Initialisiert ein neues Array.
 * 
 * Die Initialisierung des Zeigers mit `NULL` ist ebenfalls eine gültige
 * Methode, um den Vektor zu initialisieren.
 * 
 * @param self  Das Array
 */
#define vecInit(self) \
    (self = vecInit(8, sizeof(*(self))))

/**
 * @brief Gibt den Speicher eines Vektors frei.
 * @param self  Der freizugebende Vektor
 */
extern void vecRelease(void *self);

/**
 * @internal
 * @brief Reserviert Platz für einen neuen Wert im Vektor.
 * @param self  Der Vektor
 * @param size  Größe der Vektorelemente
 * @return Der neue Zeiger auf den Anfang des Vektors
 */
extern void* vecPush(void *self, size_t size);

/**
 * Fügt ein Element zum Ende des Vektors hinzu und gibt eine lvalue-Referenz
 * zurück.
 * 
 * Das Makro kann folgendermaßen verwendet werden:
```
    float *myVec = NULL;
    vecPush(myVec) = 0.5;
```
 */
#define vecPush(self) \
    (self = vecPush(self, sizeof((self)[0])), (self)+vecLen(self)-1)[0]

/**
 * @internal
 * @brief Löscht das letzte Element des Vektors.
 * @param self  Der Vektor
 */
extern void vecPop(void *self);

/**
 * Entfernt ein Element vom Ende des Vektors und gibt eine lvalue-Referenz
 * zurück.
 * 
 * Das Makro kann folgendermaßen verwendet werden:
```
    float *myVec;
    float x;

    vecInit(myVec, 4);
    vecPush(myVec) = 0.5;
    vecPush(myVec) = 1.5;
    vecPush(myVec) = 2.5;

    x = vecPop(myVec); // ergibt 2.5
```
 */
#define vecPop(self) \
    (vecPop(self), (self)+vecLen(self))[0]

/**
 * @brief Gibt das letzte Element des Vektors zurück.
 * @param self  Der Vektor
 * @return lvalue-Referenz auf das oberste Element von \p self
 */
#define vecTop(self) \
    (self)[vecLen(self) - 1]

/**
 * Durchläuft jedes Element des Vektors in Reihenfolge.
 * 
 * Das Makro kann folgendermaßen verwendet werden:
```
    float *myVec = NULL;

    vecPush(myVec) = 0.5;
    vecPush(myVec) = 1.5;
    vecPush(myVec) = 2.5;

    vecForEach(float *x, myVec) {
        // mache etwas mit x
    }
```
 * Die generierte Schleife unterstützt die Nutzung von `break` und `continue`.
 */
#define vecForEach(var, vec) \
    for (size_t _it = 0, _len = vecLen(vec), c = 0; (c ^= 1) && _it < _len; ++_it) \
        for (var = &(vec)[_it]; c != 0; c = 0)

/**
 * @brief Gibt zurück, ob der Vektor leer ist.
 * @param self  Der Vektor
 * @return 0, wenn nicht leer\n
 *      != 0, wenn leer
 */
extern int vecIsEmpty(const void *self);

/**
 * @brief Gibt die Anzahl der im Vektor verwendeten Elemente zurück.
 * @param self  Der Vektor
 * @return Anzahl der Elemente in \p self
 */
extern unsigned int vecLen(const void *self);

#endif /* VEC_H_INCLUDED */
