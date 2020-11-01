#ifndef parser
#define parser
struct line {
	char** tokens;
	int tokenscount;
	int truen;
};

struct lnarray {
	struct line** lns;
	int count;
};

struct Parser {
	FILE* input;
	struct lnarray* lns;
	int widestln;
	int maxtokens;
};

struct Parser* mkparser(FILE* input);
void freeparser(struct Parser* p);
void parse(struct Parser* p);
#endif
