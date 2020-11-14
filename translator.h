#ifndef translator
#define translator
#include "parser.h"

struct asmln {
	char* instr;
	int truen;
};

struct Translator {
	struct asmln** asmlns;
	int asmind;
	int asmsize;
	char** toclean;
	int tocleansize;
	int tocleanind;
	struct lnarray* lns;
	char* fname;
	int fnamelen;
	int compcount;
};

void freetranslator(struct Translator* t);
void printasmlns(struct Translator* t, FILE* stream);
void translate(struct Translator* t);
struct Translator* mktranslator(struct lnarray* lns, char* fname);
#endif
