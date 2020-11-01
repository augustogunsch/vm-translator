#ifndef translator
#define translator
#include "parser.h"

struct asmln {
	char* instr;
	int truen;
};

void freeasmlns();
void printasmlns();
void translate(struct line** lns, int lnscount, char* fname);
#endif
