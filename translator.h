#ifndef TRANSLATOR_H
#define TRANSLATOR_H
#include <stdbool.h>
#include "parser.h"

typedef struct {
	char** items;
	int count;
	int size;
} STRLIST;

typedef struct {
	STRLIST* asmlns;
	STRLIST* toclean;
	LINEARRAY* lns;
	char* fname;
	int fnamelen;
	char* lastfun;
	int lastfunlen;
	int funcount;
	int retind;
	int cmpind;
	bool returned;
} TRANSLATOR;

void freetranslator(TRANSLATOR* t);
void printasmlns(TRANSLATOR* t, FILE* stream);
void translate(TRANSLATOR* t);
TRANSLATOR* mktranslator(LINEARRAY* lns, char* fname);
#endif
