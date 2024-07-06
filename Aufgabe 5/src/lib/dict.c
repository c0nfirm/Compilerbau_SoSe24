/***************************************************************************//**
 * @file dict.c
 * @author Dorian Weber
 * @brief Implementation des assoziativen Arrays.
 ******************************************************************************/

#include "dict.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

/* ****************************************************** internal structures */

/**
 * @internal
 * @brief Rückgabetyp für Lokalisierungsanfragen an ein Wörterbuch.
 */
typedef struct {
	unsigned int i; /**<@brief Rückgabeindex. */
	DictEntry* p;   /**<@brief Zeiger auf das Rückgabeelement. */
} LocateResult;

/**
 * @internal
 * @brief Ganzzahlentyp für den Hashwert.
 */
typedef uint32_t Hash;

static const Hash FNVHashSeed = 0x811c9dc5u;
static const Hash FNVHashPrime = 0x1000193u;

/**
 * @internal
 * @brief Gibt zurück, ob ein Eintrag noch nie in Benutzung war.
 */
static inline int isNeverUsed(const DictEntry *entry) {
	return entry->key == ((char*) 0);
}

/**
 * @internal
 * @brief Gibt zurück, ob ein Eintrag schon zuvor in Benutzung war.
 */
static inline int isPrevUsed(const DictEntry *entry) {
	return entry->key == ((char*) -1);
}

/**
 * @internal
 * @brief Gibt zurück, ob ein Eintrag gerade nicht in Benutzung ist.
 */
static inline int isUnused(const DictEntry *entry) {
	return isNeverUsed(entry) || isPrevUsed(entry);
}

/**
 * @internal
 * @brief Setzt den Eintrag auf unbenutzt.
 */
static inline void setUnused(DictEntry *entry) {
	assert(!isUnused(entry));
	free(entry->key);
	entry->key = ((char*) -1);
}

/* ******************************************************** private functions */

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

/**
 * @internal
 * @brief Hashfunktion FNV-1a von Fowler, Noll und Vo.
 * @param key  der Schlüssel
 * @return der Hashwert von \p key
 */
static Hash fnvHash(const char *key) {
	Hash hash;
	
	for (hash = FNVHashSeed; *key != 0; ++key) {
		hash ^= *key;
		hash *= FNVHashPrime;
	}
	
	return hash;
}

/**
 * @internal
 * @brief Findet den Eintrag zu einem gegebenen Schlüssel oder die erste freie
 * Position, falls der Schlüssel nicht enthalten ist.
 * @param[in]  self    das Wörterbuch
 * @param[in]  key     der Schlüssel
 * @param[out] result  Ergebnis
 * @return 1, falls der Eintrag gefunden wurde,\n
 *         0, ansonsten
 */
static int locate(const Dict *self, const char *key, LocateResult *result) {
	enum { UNUSED, FORMER, MATCH } state = UNUSED;
	Hash hash = fnvHash(key);
	const unsigned int mask = (1u << self->bits) - 1;
	const unsigned int initial = hash & mask;
	
	/* berechne den initialen Index */
	LocateResult probe = { initial, &self->data[initial] };
	
	/* berechne die Schrittweite unter Einsatz weiterer Bits des Hashwertes;
	 * die Schrittweite entsteht aus der bitweisen Rotation des Hashwertes
	 * um die Anzahl der Bits in der Hashmap und ist immer ungerade (erstes
	 * Bit auf eins fixiert); der Grund dafür ist, dass die Größe der
	 * Hashmap eine Potenz von 2 und jede ungerade Zahl relativ prim dazu
	 * ist; mit dem euklidischen Algorithmus lässt sich zeigen, dass in
	 * diesem Fall alle Einträge in einer zufälligen Permutation besucht
	 * werden; die untere Schleife findet daher garantiert einen freien
	 * Platz, falls das Wörterbuch nicht zu 100% ausgelastet ist */
	hash = (hash >> (self->bits - 1))
	     | (hash << (sizeof(hash)*CHAR_BIT - self->bits + 1))
	     | 1;
	
	/* diese Schleife läuft, bis wir den initialen Index erneut sehen */
	do {
		/* teste, ob der aktuelle Eintrag schon in Benutzung war */
		if (isNeverUsed(probe.p)) {
			/* teste, ob wir bisher noch keinen freien Eintrag
			 * gefunden haben */
			if (state == UNUSED)
				*result = probe;
			
			/* Abbruch: der Eintrag war noch nie in Benutzung */
			break;
		}
		
		/* teste, ob der aktuelle Eintrag im Moment benutzt wird */
		if (isPrevUsed(probe.p)) {
			/* teste, ob es sich um das erste Mal handelt,
			 * dass wir einen freien Eintrag finden */
			if (state == UNUSED) {
				*result = probe;
				state = FORMER;
			}
		}
		/* teste, ob dieser Eintrag der gesuchte ist */
		else if (strcmp(probe.p->key, key) == 0) {
			*result = probe;
			state = MATCH;
			break;
		}
		
		/* gehe zum nächsten Index */
		probe.i = (probe.i + hash) & mask;
		probe.p = &self->data[probe.i];
	}
	while (probe.i != initial);
	
	return state == MATCH;
}

/**
 * @internal
 * @brief Verdoppelt die Größe des Wörterbuchs und hasht alle Werte neu.
 * @param self  das Wörterbuch
 */
static void grow(Dict *self) {
	DictEntry* data = self->data, *it, *end;
	unsigned int cap = 1u << self->bits;
	
	/* alloziere ein doppelt so großes Array für die Einträge */
	self->data = calloc(2*cap, sizeof(*self->data));
	
	/* teste auf Speicherfehler */
	if (self->data == NULL) {
		fputs("out-of-memory error\n", stderr);
		exit(-1);
	}
	
	/* aktualisiere die Anzahl der Bits und Plätze;
	 * ziele auf eine Füllrate von 75% */
	++self->bits;
	self->left += cap / 4 * 3;
	
	/* iteriere über das Originalarray und füge die Elemente neu ein */
	for (it = data, end = it + cap; it < end; ++it) {
		if (!isUnused(it)) {
			LocateResult loc;
			
			locate(self, it->key, &loc);
			*loc.p = *it;
		}
	}
	
	/* gib' das alte Array frei */
	free(data);
}

/* ********************************************************* public functions */

void dictInit(Dict *self) {
	unsigned int cap;
	
	/* reserviere Platz für die Schlüssel-Wert-Paare */
	self->bits = 3;
	cap = 1u << self->bits;
	self->data = calloc(cap, sizeof(*self->data));
	
	/* ziele auf eine Füllrate von 75% */
	self->left = cap / 4 * 3;
	
	if (self->data == NULL) {
		fputs("out-of-memory error\n", stderr);
		exit(-1);
	}
}

void dictRelease(Dict *self) {
	DictEntry *it, *end;
	
	/* iteriere über das Array und gib' Schlüssel benutzter Elemente frei */
	for (it = self->data, end = it + (1u << self->bits); it < end; ++it) {
		if (!isUnused(it))
			free(it->key);
	}
	
	free(self->data);
}

unsigned int dictInsert(Dict *self, const char *key, unsigned int val) {
	unsigned int oldVal = -1u;
	LocateResult loc;
	
	assert(key != NULL && val != -1u);
	
	/* vergrößere die Hashmap bei (potenziellem) Platzmangel */
	if (self->left == 0)
		grow(self);
	
	/* finde den Eintrag */
	if (locate(self, key, &loc)) {
		/* überschreibe den aktuellen Wert */
		oldVal = loc.p->val;
		loc.p->val = val;
	} else {
		/* erstelle den Wert neu */
		loc.p->key = stringDup(key);
		loc.p->val = val;
		--self->left;
	}
	
	/* gib den vorigen Wert zurück */
	return oldVal;
}

unsigned int dictGet(const Dict *self, const char *key) {
	LocateResult loc;
	
	if (locate(self, key, &loc))
		return loc.p->val;
	
	return -1u;
}

unsigned int dictRemove(Dict *self, const char *key) {
	unsigned int val = (unsigned int) -1;
	LocateResult loc;
	
	if (locate(self, key, &loc)) {
		val = loc.p->val;
		setUnused(loc.p);
		++self->left;
	}
	
	return val;
}
