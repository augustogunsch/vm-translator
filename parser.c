#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "parser.h"

void freelns(struct lnarray* lns) {
	for(int i = 0; i < lns->count; i++) {
		int tkcount = lns->lns[i]->tokenscount;
		for(int j = 0; j < tkcount; j++) {
			free(lns->lns[i]->tokens[j]);
		}
		free(lns->lns[i]->tokens);
		free(lns->lns[i]);
	}
	free(lns->lns);
	free(lns);
}

void freeparser(struct Parser* p) {
	freelns(p->lns);
	free(p);
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

void getinfo(struct Parser* p) {
	p->lns->count = 0;
	p->maxtokens = 0;
	p->widestln = 0;
	char c, nc;
	int widest = 0;
	int currsz = 0;
	int tokens = 0;
	short readsmt = 0;
	while(c = fgetc(p->input), c != -1) {
		currsz++;
		if(isspace(c)) {
			if(readsmt) {
				tokens++;
				readsmt = 0;
			}
			if(c == '\n' && tokens > 0) {
				p->lns->count++;
				if(currsz > p->widestln)
					p->widestln = currsz;
				if(tokens > p->maxtokens)
					p->maxtokens = tokens;
				currsz = 0;
				tokens = 0;
			}
			continue;
		}

		if(c == '/') {
			nc = fgetc(p->input);
			if(nc == '/') {
				gountilbrk(p->input);
				continue;
			}
			ungetc(nc, p->input);
		}

		readsmt = 1;
	}
	rewind(p->input);
}

struct Parser* mkparser(FILE* input) {
	struct Parser* p = (struct Parser*)malloc(sizeof(struct Parser));
	struct lnarray* lns = (struct lnarray*)malloc(sizeof(struct lnarray));
	p->input = input;
	p->lns = lns;
	getinfo(p);
	return p;
}

void parse(struct Parser* p) {
	struct line** lns = (struct line**)malloc(sizeof(struct line*)*p->lns->count);
	p->lns->lns = lns;
	p->lns->count = 0;

	char tmp[p->widestln];
	char* tokens[p->maxtokens];

	char c, nc;
	short readsmt = 0;
	int tmpind = 0;
	int tokensind = 0;
	int truelncount = 0;

	while(c = fgetc(p->input), c != -1) {
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
					lns[p->lns->count] = newln;
					p->lns->count++;
					tokensind = 0;
				}
			}
			continue;
		}
		
		if(c == '/') {
			nc = fgetc(p->input);
			if(nc == '/') {
				gountilbrk(p->input);
				continue;
			}
			ungetc(nc, p->input);
		}
		
		tmp[tmpind] = c;
		tmpind++;
		readsmt = 1;
	}
	fclose(p->input);
}
