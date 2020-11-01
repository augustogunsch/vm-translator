#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "translator.h"
#include "templates.h"

struct asmln** asmlns;
int asmind;
int asmsize;
char** toclean;
int tocleansize;
int tocleanind;

void pushtoclean(char* topush) {
	int nextsz = sizeof(char*)*(tocleanind+1);
	if(nextsz >= tocleansize) {
		tocleansize = nextsz * 2;
		toclean = realloc(toclean, tocleansize);
	}
	toclean[tocleanind] = topush;
	tocleanind++;
}

void freetoclean() {
	for(int i = 0; i < tocleanind; i++) {
		free(toclean[i]);
	}
	free(toclean);
	tocleansize = 0;
	tocleanind = 0;
}

void freeasmlns() {
	for(int i = 0; i < asmind; i++) {
		free(asmlns[i]);
	}
	free(asmlns);
	asmsize = 0;
	asmind = 0;
	freetoclean();
}

void printasmlns() {
	for(int i = 0; i < asmind; i++) {
		printf("%s\n", asmlns[i]->instr);
	}
}

char* heapstr(const char* input) {
	char* newstr = (char*)malloc(sizeof(char)*(strlen(input)+1));
	strcpy(newstr, input);
	pushtoclean(newstr);
	return newstr;
}

char* switchseg(struct line* ln) {
	char* seg = ln->tokens[1];
	if(strcmp(seg, "local") == 0)
		return heapstr("@LCL");
	if(strcmp(seg, "argument") == 0)
		return heapstr("@ARG");
	if(strcmp(seg, "this") == 0)
		return heapstr("@THIS");
	if(strcmp(seg, "that") == 0)
		return heapstr("@THAT");
	fprintf(stderr, "Unrecognized segment '%s'; line %i\n", seg, ln->truen);
	exit(1);
}

// produce comment as follows:
// pop/push segment i
char* mkcom(struct line* ln, int indlen) {
	int comlen = sizeof(char) * (strlen(ln->tokens[0]) + strlen(ln->tokens[1]) + indlen + 6);
	char* comment = (char*)malloc(comlen);
	snprintf(comment, comlen, "// %s %s %s", ln->tokens[0], ln->tokens[1], ln->tokens[2]);
	pushtoclean(comment);
	return comment;
}

void checkind(struct line* ln, int indsz) {
	for(int i = 0; i < indsz; i++) {
		if(!isdigit(ln->tokens[2][i])) {
			fprintf(stderr, "Invalid index '%s'; line %i\n", ln->tokens[2], ln->truen);
			exit(1);
		}
	}
}

char* mkind(struct line* ln, int indsz) {
	int newsz = sizeof(char) * (indsz + 2);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%s", ln->tokens[2]);
	pushtoclean(newind);
	return newind;
}

char* mkstatind(struct line* ln, int indsz, char* fname) {
	int fnamelen = strlen(fname);
	int newsz = sizeof(char) * (fnamelen + indsz + 3);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%s.%s", fname, ln->tokens[2]);
	pushtoclean(newind);
	return newind;
}

char* mktempind(struct line* ln, int indsz) {
	int intind = atoi(ln->tokens[2]);
	int newsz = sizeof(char) * (indsz + 3);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%i", intind+5);
	pushtoclean(newind);
	return newind;
}

void checkasmsize(int toadd) {
	int targ = sizeof(struct asmln*)*(asmind+toadd);
	if(targ >= asmsize) {
		asmsize = targ * 2;
		asmlns = (struct asmln**)realloc(asmlns, asmsize);
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

void addasmlns(struct line* ln, char** insts, int instcount) {
	for(int i = 0; i < instcount; i++) {
		asmlns[asmind] = mkasmln(ln, insts[i]);
		asmind++;
	}
}

void startpoppush(struct line* ln, int indlen, char** insts) {
	// // operation segment i
	insts[0] = mkcom(ln, indlen);
	
	// @segment
	insts[1] = switchseg(ln);
	
	// D=M
	insts[2] = heapstr("D=M");

	// @i
	checkind(ln, indlen);
	insts[3] = mkind(ln, indlen);
}

void pushcons(struct line* ln, int indlen) {
	checkasmsize(TPUSHCONSN);

	// // push constant i
	tpushcons[0] = mkcom(ln, indlen);

	// @i
	checkind(ln, indlen);
	tpushcons[1] = mkind(ln, indlen);

	addasmlns(ln, tpushcons, TPUSHCONSN);
}

void pushstat(struct line* ln, int indlen, char* fname) {
	checkasmsize(TPUSHSTATN);

	// // push static i
	tpushstat[0] =  mkcom(ln, indlen);

	// @fname.i
	checkind(ln, indlen);
	tpushstat[1] = mkstatind(ln, indlen, fname);

	addasmlns(ln, tpushstat, TPUSHSTATN);
}

void pushtemp(struct line* ln, int indlen) {
	checkasmsize(TPUSHSTATN);

	// // pop static i
	tpushtemp[0] = mkcom(ln, indlen);
	
	// @5+i
	checkind(ln, indlen);
	tpushtemp[1] = mktempind(ln, indlen);

	addasmlns(ln, tpushtemp, TPUSHTEMP);
}

void mkpush(struct line* ln, int indlen) {
	checkasmsize(TPUSHN);

	startpoppush(ln, indlen, tpush);

	addasmlns(ln, tpush, TPUSHN);
}

void popstat(struct line* ln, int indlen, char* fname) {
	checkasmsize(TPOPSTATN);

	// // pop static i
	tpopstat[0] = mkcom(ln, indlen);

	// @fname.i
	checkind(ln, indlen);
	tpopstat[6] = mkstatind(ln, indlen, fname);

	// M=D
	tpopstat[7] = heapstr("M=D");

	addasmlns(ln, tpopstat, TPOPSTATN);
}

void poptemp(struct line* ln, int indlen) {
	checkasmsize(TPOPTEMPN);

	// // pop static i
	tpoptemp[0] = mkcom(ln, indlen);
	
	// @5+i
	checkind(ln, indlen);
	tpoptemp[5] = mktempind(ln, indlen);

	// M=D
	tpoptemp[6] = heapstr("M=D");

	addasmlns(ln, tpoptemp, TPOPTEMPN);
}

void mkpop(struct line* ln, int indlen) {
	checkasmsize(TPOPN);

	startpoppush(ln, indlen, tpop);

	addasmlns(ln, tpop, TPOPN);
}

void switchop(struct line* ln, char* fname) {
	char* op = ln->tokens[0];

	if(strcmp(op, "push") == 0) {
		checkopamnt(3, ln);
		char* seg = ln->tokens[1];
		int indlen = strlen(ln->tokens[2]);

		if(strcmp(seg, "constant") == 0)
			pushcons(ln, indlen);

		else if(strcmp(seg, "static") == 0)
			pushstat(ln, indlen, fname);

		else if(strcmp(seg, "temp") == 0)
			pushtemp(ln, indlen);

		else
			mkpush(ln, indlen);
	}
	else if(strcmp(op, "pop") == 0) {
		checkopamnt(3, ln);
		char* seg = ln->tokens[1];
		int indlen = strlen(ln->tokens[2]);

		if(strcmp(seg, "static") == 0)
			popstat(ln, indlen, fname);

		else if(strcmp(seg, "temp") == 0)
			poptemp(ln, indlen);

		else
			mkpop(ln, indlen);
	}
}

void translate(struct line** lns, int lnscount, char* fname) {
	asmsize = sizeof(struct asmln*)*(lnscount * 15);
	asmind = 0;
	asmlns = (struct asmln**)malloc(asmsize);

	tocleanind = 0;
	tocleansize = sizeof(char*)*(lnscount * 5);
	toclean = (char**)malloc(tocleansize);
	
	for(int i = 0; i < lnscount; i++) {
		struct line* ln = lns[i];
		switchop(ln, fname);
	}
}
