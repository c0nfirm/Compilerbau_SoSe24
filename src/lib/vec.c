/***************************************************************************//**
 * @file vec.c
 * @author Dorian Weber
 * @brief Implementation of the dynamically sized vector.
 ******************************************************************************/

#include "vec.h"
#include <stdio.h>
#include <stdlib.h>

/* ********************************************************* public functions */

/* (die runden Klammern um einige Funktionsnamen sind notwendig, da Makros
 * gleichen Namens existieren und der Präprozessor diese expandieren würde) */

void*
(vecInit)(size_t capacity, size_t size) {
	VecHeader *hdr = malloc(sizeof(*hdr) + size*capacity);
	
	if (hdr == NULL) {
		fputs("out-of-memory error\n", stderr);
		exit(-1);
	}
	
	hdr->len = 0;
	hdr->cap = capacity;
	
	return hdr + 1;
}

void
vecRelease(void *self) {
	free(((VecHeader*) self) - 1);
}

void*
(vecPush)(void *self, size_t size) {
	VecHeader *hdr = ((VecHeader*) self) - 1;
	
	if (hdr->len == hdr->cap) {
		hdr->cap *= 2;
		hdr = realloc(hdr, sizeof(*hdr) + size*hdr->cap);
		
		if (hdr == NULL) {
			fputs("out-of-memory error\n", stderr);
			exit(-1);
		}
	}
	
	++hdr->len;
	return hdr + 1;
}

void
(vecPop)(void *self) {
	VecHeader *hdr = ((VecHeader*) self) - 1;
	
	if (hdr->len <= 0) {
		fputs("attempted to pop an empty vector\n", stderr);
		exit(-1);
	}
	
	--hdr->len;
}

int
vecIsEmpty(const void *self) {
	return ((VecHeader*) self)[-1].len == 0;
}

unsigned int
vecLen(const void *self) {
	return ((VecHeader*) self)[-1].len;
}
