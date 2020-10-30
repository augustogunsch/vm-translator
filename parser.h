#ifndef parser
#define parser
struct line {
	char** tokens;
	int tokenscount;
	int truen;
};

void getinfo(FILE* input, int* lncount, int* widestln, int* maxtokens);
void freelns(struct line** lns, int lnscount);
struct line** parse(FILE* input, int lncount, int widestln, int maxtokens);
#endif
