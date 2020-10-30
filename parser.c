#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "parser.h"

void freelns(struct line** lns, int lnscount) {
	for(int i = 0; i < lnscount; i++) {
		int tkcount = lns[i]->tokenscount;
		for(int j = 0; j < tkcount; j++) {
			free(lns[i]->tokens[j]);
		}
		free(lns[i]->tokens);
		free(lns[i]);
	}
	free(lns);
}

void gountilbrk (FILE* input) {
	char c;
	while(c = fgetc(input), c != -1) {
		if(c == '\n') {
			ungetc(c, input);
			return;
		}
	}
}

void getinfo(FILE* input, int* lncount, int* widestln, int* maxtokens) {
	char c, nc;
	int lns = 0;
	int widest = 0;
	int currsz = 0;
	int tokens = 0;
	int maxtoks = 0;
	short readsmt = 0;
	while(c = fgetc(input), c != -1) {
		currsz++;
		if(isspace(c)) {
			if(readsmt) {
				tokens++;
				readsmt = 0;
			}
			if(c == '\n' && tokens > 0) {
				lns++;
				if(currsz > widest)
					widest = currsz;
				if(tokens > maxtoks)
					maxtoks = tokens;
				currsz = 0;
				tokens = 0;
			}
			continue;
		}

		if(c == '/') {
			nc = fgetc(input);
			if(nc == '/') {
				gountilbrk(input);
				continue;
			}
			ungetc(nc, input);
		}

		readsmt = 1;
	}
	rewind(input);
	(*lncount) = lns;
	(*widestln) = widest;
	(*maxtokens) = maxtoks;
}

struct line** parse(FILE* input, int lncount, int widestln, int maxtokens) {
	struct line** lns = (struct line**)malloc(sizeof(struct line*)*lncount);
	char c, nc;
	short readsmt = 0;
	char tmp[widestln];
	char* tokens[maxtokens];
	int tmpind = 0;
	int lnsind = 0;
	int tokensind = 0;
	int truelncount = 0;
	while(c = fgetc(input), c != -1) {
		if(isspace(c)) {
			if(readsmt) {
				tmp[tmpind] = '\0';
				char* newtoken = (char*)malloc(sizeof(char)*tmpind+1);
				strcpy(newtoken, tmp);
				tokens[tokensind] = newtoken;
				tmpind = 0;
				readsmt = 0;
				tokensind++;
			}
			if(c == '\n') {
				truelncount++;
				if(tokensind > 0) {
					struct line* newln = (struct line*)malloc(sizeof(struct line));
					newln->tokens = (char**)malloc(sizeof(char*)*tokensind);
					for(int i = 0; i < tokensind; i++) {
						newln->tokens[i] = tokens[i];
					}
					newln->tokenscount = tokensind;
					newln->truen = truelncount;
					lns[lnsind] = newln;
					lnsind++;
					tokensind = 0;
				}
			}
			continue;
		}
		
		if(c == '/') {
			nc = fgetc(input);
			if(nc == '/') {
				gountilbrk(input);
				continue;
			}
			ungetc(nc, input);
		}
		
		tmp[tmpind] = c;
		tmpind++;
		readsmt = 1;
	}
	return lns;
}
