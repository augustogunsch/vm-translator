#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "translator.h"
#include "templates.h"

void pushtoclean(struct Translator* t, char* topush) {
	int nextsz = sizeof(char*)*(t->tocleanind+1);
	if(nextsz >= t->tocleansize) {
		t->tocleansize = nextsz * 2;
		t->toclean = realloc(t->toclean, t->tocleansize);
	}
	t->toclean[t->tocleanind] = topush;
	t->tocleanind++;
}

void freetoclean(struct Translator* t) {
	for(int i = 0; i < t->tocleanind; i++)
		free(t->toclean[i]);
	free(t->toclean);
	t->tocleansize = 0;
	t->tocleanind = 0;
}

void freeasmlns(struct Translator* t) {
	for(int i = 0; i < t->asmind; i++)
		free(t->asmlns[i]);
	free(t->asmlns);
	t->asmsize = 0;
	t->asmind = 0;
}

void freetranslator(struct Translator* t) {
	freeasmlns(t);
	freetoclean(t);
	free(t);
}

void printasmlns(struct Translator* t) {
	for(int i = 0; i < t->asmind; i++)
		printf("%s\n", t->asmlns[i]->instr);
}

char* heapstr(struct Translator* t, const char* input) {
	char* newstr = (char*)malloc(sizeof(char)*(strlen(input)+1));
	strcpy(newstr, input);
	pushtoclean(t, newstr);
	return newstr;
}

char* switchseg(struct Translator* t, struct line* ln) {
	char* seg = ln->tokens[1];
	if(strcmp(seg, "local") == 0)
		return heapstr(t, "@LCL");
	if(strcmp(seg, "argument") == 0)
		return heapstr(t, "@ARG");
	if(strcmp(seg, "this") == 0)
		return heapstr(t, "@THIS");
	if(strcmp(seg, "that") == 0)
		return heapstr(t, "@THAT");
	fprintf(stderr, "Unrecognized segment '%s'; line %i\n", seg, ln->truen);
	exit(1);
}

// produce comment as follows:
// pop/push segment i
char* mkcom(struct Translator* t, struct line* ln, int indlen) {
	int comlen = sizeof(char) * (strlen(ln->tokens[0]) + strlen(ln->tokens[1]) + indlen + 6);
	char* comment = (char*)malloc(comlen);
	snprintf(comment, comlen, "// %s %s %s", ln->tokens[0], ln->tokens[1], ln->tokens[2]);
	pushtoclean(t, comment);
	return comment;
}

void checkind(struct line* ln, int indsz) {
	for(int i = 0; i < indsz; i++)
		if(!isdigit(ln->tokens[2][i])) {
			fprintf(stderr, "Invalid index '%s'; line %i\n", ln->tokens[2], ln->truen);
			exit(1);
		}
}

char* mkind(struct Translator* t, struct line* ln, int indsz) {
	int newsz = sizeof(char) * (indsz + 2);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%s", ln->tokens[2]);
	pushtoclean(t, newind);
	return newind;
}

char* mkstatind(struct Translator* t, struct line* ln, int indsz) {
	int fnamelen = strlen(t->fname);
	int newsz = sizeof(char) * (fnamelen + indsz + 3);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%s.%s", t->fname, ln->tokens[2]);
	pushtoclean(t, newind);
	return newind;
}

char* mktempind(struct Translator* t, struct line* ln, int indsz) {
	int intind = atoi(ln->tokens[2]);
	int newsz = sizeof(char) * (indsz + 3);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%i", intind+5);
	pushtoclean(t, newind);
	return newind;
}

void checkasmsize(struct Translator* t, int toadd) {
	int targ = sizeof(struct asmln*)*(t->asmind+toadd);
	if(targ >= t->asmsize) {
		t->asmsize = targ * 2;
		t->asmlns = (struct asmln**)realloc(t->asmlns, t->asmsize);
	}
}

struct asmln* mkasmln(struct line* ln, char* content) {
	struct asmln* newln = (struct asmln*)malloc(sizeof(struct asmln));
	newln->truen = ln->truen;
	newln->instr = content;
	return newln;
}

void checkopamnt(int amnt, struct line* ln) {
	if(ln->tokenscount < 2) {
		fprintf(stderr, "Missing memory segment; line %i", ln->truen);
		exit(1);
	}
	if(amnt > 2)
		if(ln->tokenscount < 3) {
			fprintf(stderr, "Missing operation index; line %i", ln->truen);
			exit(1);
		}
}

void addasmlns(struct Translator* t, struct line* ln, char** insts, int instcount) {
	for(int i = 0; i < instcount; i++) {
		t->asmlns[t->asmind] = mkasmln(ln, insts[i]);
		t->asmind++;
	}
}

void startpoppush(struct Translator* t, struct line* ln, int indlen, char** insts) {
	// // operation segment i
	insts[0] = mkcom(t, ln, indlen);
	
	// @segment
	insts[1] = switchseg(t, ln);
	
	// D=M
	insts[2] = heapstr(t, "D=M");

	// @i
	checkind(ln, indlen);
	insts[3] = mkind(t, ln, indlen);
}

void pushcons(struct Translator* t, struct line* ln, int indlen) {
	checkasmsize(t, TPUSHCONSN);

	// // push constant i
	tpushcons[0] = mkcom(t, ln, indlen);

	// @i
	checkind(ln, indlen);
	tpushcons[1] = mkind(t, ln, indlen);

	addasmlns(t, ln, tpushcons, TPUSHCONSN);
}

void pushstat(struct Translator* t, struct line* ln, int indlen) {
	checkasmsize(t, TPUSHSTATN);

	// // push static i
	tpushstat[0] =  mkcom(t, ln, indlen);

	// @fname.i
	checkind(ln, indlen);
	tpushstat[1] = mkstatind(t, ln, indlen);

	addasmlns(t, ln, tpushstat, TPUSHSTATN);
}

void pushtemp(struct Translator* t, struct line* ln, int indlen) {
	checkasmsize(t, TPUSHSTATN);

	// // pop static i
	tpushtemp[0] = mkcom(t, ln, indlen);
	
	// @5+i
	checkind(ln, indlen);
	tpushtemp[1] = mktempind(t, ln, indlen);

	addasmlns(t, ln, tpushtemp, TPUSHTEMP);
}

void push(struct Translator* t, struct line* ln, int indlen) {
	checkasmsize(t, TPUSHN);

	startpoppush(t, ln, indlen, tpush);

	addasmlns(t, ln, tpush, TPUSHN);
}

void popstat(struct Translator* t, struct line* ln, int indlen) {
	checkasmsize(t, TPOPSTATN);

	// // pop static i
	tpopstat[0] = mkcom(t, ln, indlen);

	// @fname.i
	checkind(ln, indlen);
	tpopstat[6] = mkstatind(t, ln, indlen);

	// M=D
	tpopstat[7] = heapstr(t, "M=D");

	addasmlns(t, ln, tpopstat, TPOPSTATN);
}

void poptemp(struct Translator* t, struct line* ln, int indlen) {
	checkasmsize(t, TPOPTEMPN);

	// // pop static i
	tpoptemp[0] = mkcom(t, ln, indlen);
	
	// @5+i
	checkind(ln, indlen);
	tpoptemp[5] = mktempind(t, ln, indlen);

	// M=D
	tpoptemp[6] = heapstr(t, "M=D");

	addasmlns(t, ln, tpoptemp, TPOPTEMPN);
}

void pop(struct Translator* t, struct line* ln, int indlen) {
	checkasmsize(t, TPOPN);

	startpoppush(t, ln, indlen, tpop);

	addasmlns(t, ln, tpop, TPOPN);
}

void switchop(struct Translator* t, struct line* ln) {
	char* op = ln->tokens[0];

	if(strcmp(op, "push") == 0) {
		checkopamnt(3, ln);
		char* seg = ln->tokens[1];
		int indlen = strlen(ln->tokens[2]);

		if(strcmp(seg, "constant") == 0)
			pushcons(t, ln, indlen);

		else if(strcmp(seg, "static") == 0)
			pushstat(t, ln, indlen);

		else if(strcmp(seg, "temp") == 0)
			pushtemp(t, ln, indlen);

		else
			push(t, ln, indlen);
	}
	else if(strcmp(op, "pop") == 0) {
		checkopamnt(3, ln);
		char* seg = ln->tokens[1];
		int indlen = strlen(ln->tokens[2]);

		if(strcmp(seg, "static") == 0)
			popstat(t, ln, indlen);

		else if(strcmp(seg, "temp") == 0)
			poptemp(t, ln, indlen);

		else
			pop(t, ln, indlen);
	}
}

void translate(struct Translator* t) {
	for(int i = 0; i < t->lns->count; i++)
		switchop(t, t->lns->lns[i]);
}

struct Translator* mktranslator(struct lnarray* lns, char* fname) {
	struct Translator* t = (struct Translator*)malloc(sizeof(struct Translator));
	t->asmsize = sizeof(struct asmln*)*(lns->count * 15);
	t->asmind = 0;
	t->asmlns = (struct asmln**)malloc(t->asmsize);

	t->tocleanind = 0;
	t->tocleansize = sizeof(char*)*(lns->count * 5);
	t->toclean = (char**)malloc(t->tocleansize);

	t->lns = lns;
	t->fname = fname;

	return t;
}
