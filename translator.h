#ifndef translator
#define translator
#include "parser.h"

struct Translator {
	char** asmlns;
	int asmind;
	int asmsize;
	char** toclean;
	int tocleansize;
	int tocleanind;
	struct lnarray* lns;
	char* fname;
	int fnamelen;
	char* lastfun;
	int lastfunlen;
	int funcount;
	int retind;
	int cmpind;
	short returned;
};

void freetranslator(struct Translator* t);
void printasmlns(struct Translator* t, FILE* stream);
void translate(struct Translator* t);
struct Translator* mktranslator(struct lnarray* lns, char* fname);
#endif
