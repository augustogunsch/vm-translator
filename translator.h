#ifndef translator
#define translator
#include "parser.h"
struct asmln {
	char* instr;
	int truen;
};

struct asmln** translate(struct line** lns, int lnscount, int* asmind);
#endif
