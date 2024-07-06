/***************************************************************************//**
 * @file dict.h
 * @author Dorian Weber
 * @brief Enthält die Schnittstelle für ein assoziatives Array (Wörterbuch).
 * @details
 * Hier ist ein Beispiel für die Benutzung des Wörterbuches:
 * @code
 * Dict dict;
 * 
 * dictInit(&dict);
 * 
 * dictInsert(&dict, "One", 1);
 * dictInsert(&dict, "Two", 2);
 * dictInsert(&dict, "Three", 3);
 * 
 * printf("One -> %u\n", dictGet(&dict, "One"));
 * printf("Two -> %u\n", dictGet(&dict, "Two"));
 * printf("Three -> %u\n", dictGet(&dict, "Three"));
 * 
 * dictInsert(&dict, "One", 0);
 * dictRemove(&dict, "Two");
 * 
 * printf("One -> %u\n", dictGet(&dict, "One"));
 * printf("Two -> %u\n", dictGet(&dict, "Two"));
 * printf("Three -> %u\n", dictGet(&dict, "Three"));
 * 
 * dictRelease(&dict);
 * @endcode
 * mit der Ausgabe
 * @code
 * One -> 1
 * Two -> 2
 * Three -> 3
 * One -> 0
 * Two -> 4294967295
 * Three -> 3
 * @endcode
 ******************************************************************************/

#ifndef DICT_H_INCLUDED
#define DICT_H_INCLUDED

/* *** structures *********************************************************** */

/**
 * @brief Wörterbucheintrag.
 */
typedef struct {
	char *key;        /**<@brief Der Schlüssel. */
	unsigned int val; /**<@brief Der Wert. */
} DictEntry;

/**
 * @brief Wörterbuch.
 */
typedef struct {
	/**
	 * @brief Datenfeld für Schlüssel-/Wertpaare.
	 */
	DictEntry *data;
	
	/**
	 * @brief Anzahl der Elemente, die eingefügt werden können, bevor neuer
	 * Platz benötigt wird.
	 */
	unsigned int left;
	
	/**
	 * @brief Anzahl der Bits für die Kapazität des Datenfelds.
	 * 
	 * Es wird statt der Kapazität die Anzahl der verwendeten Bits
	 * gespeichert, da die Größe der Hashmap per Konstruktion eine Potenz
	 * von zwei ist. Dadurch vereinfacht sich bitweise Rotation in der
	 * Lokalisationsroutine. Die tatsächliche Kapazität findet man via
	 * `1 << bits`.
	 */
	unsigned int bits;
} Dict;

/* *** interface ************************************************************ */

/**
 * @brief Initialisiert das Wörterbuch.
 * @param self  das Wörterbuch
 */
extern void dictInit(Dict *self);

/**
 * @brief Gibt ein Wörterbuch und alle darin gespeicherten Schlüssel frei.
 * @param self  das Wörterbuch
 */
extern void dictRelease(Dict *self);

/**
 * @brief Assoziiert einen Schlüssel mit einem Wert.
 * @param self    das Wörterbuch
 * @param key     der Schlüssel
 * @param val     der neue Wert
 * @return der alte Wert, falls er überschrieben wurde,
 *         `-1u`, ansonsten
 */
extern unsigned int dictInsert(Dict *self, const char *key, unsigned int val);

/**
 * @brief Gibt den Wert zu einem Schlüssel zurück.
 * @param self  das Wörterbuch
 * @param key   der Schlüssel
 * @return der assoziierte Wert, falls der Schlüssel enthalten ist,
 *         `-1u`, ansonsten
 */
extern unsigned int dictGet(const Dict *self, const char *key);

/**
 * @brief Entfernt eine Schlüssel-/Wert-Assoziation.
 * @param self  das Wörterbuch
 * @param key   der Schlüssel
 * @return der entfernte Wert, falls der Schlüssel enthalten ist,
 *         `-1u`, ansonsten
 */
extern unsigned int dictRemove(Dict *self, const char *key);

#endif /* DICT_H_INCLUDED */
