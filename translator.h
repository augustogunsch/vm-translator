#ifndef translator
#define translator
#include "parser.h"
struct asmln {
	char* instr;
	int truen;
};

void freeasmlns(struct asmln** lns, int count);
struct asmln** translate(struct line** lns, int lnscount, int* asmcount, char* fname);
#endif
