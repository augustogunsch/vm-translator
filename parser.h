#ifndef PARSER_H
#define PARSER_H
typedef struct {
	char** tokens;
	int tokenscount;
	int truen;
} LINE;

typedef struct {
	LINE** lns;
	int count;
} LINEARRAY;

typedef struct {
	FILE* input;
	LINEARRAY* lns;
	int widestln;
	int maxtokens;
} PARSER;

PARSER* mkparser(FILE* input);
void freeparser(PARSER* p);
void parse(PARSER* p);
#endif
