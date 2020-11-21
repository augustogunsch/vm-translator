#ifndef TRANSLATOR_H
#define TRANSLATOR_H
#include <stdbool.h>
#include "parser.h"
#include "util.h"

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

	LINELIST* output;
	LINELIST* curln;
	LINELIST* lastln;
	int lncount;
} TRANSLATOR;

void freetranslator(TRANSLATOR* t);
void translate(TRANSLATOR* t);
TRANSLATOR* mktranslator(LINEARRAY* lns, char* fname);
#endif
