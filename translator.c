#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "translator.h"
#include "templates.h"
#define CMPLIMIT 9999
#define CMPLEN 4

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

void printasmlns(struct Translator* t, FILE* stream) {
	for(int i = 0; i < t->asmind; i++)
		fprintf(stream, "%s\n", t->asmlns[i]->instr);
}

char* heapstr(struct Translator* t, const char* input) {
	char* newstr = (char*)malloc(sizeof(char)*(strlen(input)+1));
	strcpy(newstr, input);
	pushtoclean(t, newstr);
	return newstr;
}

char* switchseg(struct Translator* t, struct line* ln) {
	char* seg = ln->tokens[1];
	if(!strcmp(seg, "local"))
		return heapstr(t, "@LCL");
	if(!strcmp(seg, "argument"))
		return heapstr(t, "@ARG");
	if(!strcmp(seg, "this"))
		return heapstr(t, "@THIS");
	if(!strcmp(seg, "that"))
		return heapstr(t, "@THAT");
	fprintf(stderr, "Unrecognized segment '%s'; line %i\n", seg, ln->truen);
	exit(1);
}

int lnlen(int* out, struct line* ln) {
	int len = 0;
	for(int i = 0; i < ln->tokenscount; i++) {
		int l = strlen(ln->tokens[i]);
		out[i] = l;
		len += l;
	}
	return len;
}

// produce comment as follows:
// pop/push segment i
char* mkcom(struct Translator* t, struct line* ln) {
	int lens[ln->tokenscount];
	int comlen = sizeof(char) * lnlen(lens, ln) + ln->tokenscount + 3;
	char* comment = (char*)malloc(comlen);

	comment[0] = '/';
	comment[1] = '/';

	char* tmp = comment + sizeof(char)*2;
	for(int i = 0; i < ln->tokenscount; i++) {
		tmp[0] = ' ';
		tmp += sizeof(char);
		strcpy(tmp, ln->tokens[i]);
		tmp += sizeof(char)*lens[i];
	}

	tmp[0] = '\0';

	pushtoclean(t, comment);
	return comment;
}

void checkind(struct line* ln, int indlen) {
	for(int i = 0; i < indlen; i++)
		if(!isdigit(ln->tokens[2][i])) {
			fprintf(stderr, "Invalid index '%s'; line %i\n", ln->tokens[2], ln->truen);
			exit(1);
		}
}

char* mkcmplab(struct Translator* t, struct line* ln) {
	t->compcount++;
	if(t->compcount > CMPLIMIT) {
		fprintf(stderr, "Reached comparison limit (%i); line %i\n", CMPLIMIT, ln->truen);
		exit(1);
	}
	int newsz = (t->fnamelen + CMPLEN + 6) * sizeof(char);
	char* label = (char*)malloc(newsz);
	snprintf(label, newsz, "%s-CMP-%i", t->fname, t->compcount);
	pushtoclean(t, label);
	return label;
}

char* mkind(struct Translator* t, struct line* ln, int indlen) {
	checkind(ln, indlen);
	int newsz = sizeof(char) * (indlen + 2);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%s", ln->tokens[2]);
	pushtoclean(t, newind);
	return newind;
}

char* atlab(struct Translator* t, char* label, int labellen) {
	int newsz = sizeof(char) * (labellen + 2);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%s", label);
	pushtoclean(t, newind);
	return newind;
}

char* mklab(struct Translator* t, char* label, int labellen) {
	int newsz = sizeof(char) * (labellen + 3);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "(%s)", label);
	pushtoclean(t, newind);
	return newind;
}

char* mkstatind(struct Translator* t, struct line* ln, int indlen) {
	checkind(ln, indlen);
	int newsz = sizeof(char) * (t->fnamelen + indlen + 3);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%s.%s", t->fname, ln->tokens[2]);
	pushtoclean(t, newind);
	return newind;
}

char* mktempind(struct Translator* t, struct line* ln, int indlen) {
	checkind(ln, indlen);
	int intind = atoi(ln->tokens[2]);
	int newsz = sizeof(char) * (indlen + 3);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%i", intind+5);
	pushtoclean(t, newind);
	return newind;
}

char* mkpointerind(struct Translator* t, struct line* ln, int indlen) {
	if(indlen > 1) {
		fprintf(stderr, "Invalid index '%s'; line %i\n", ln->tokens[2], ln->truen);
		exit(1);
	}
	char* ptr;
	switch(ln->tokens[2][0]) {
		case '0':
			ptr = "THIS";
			break;
		case '1':
			ptr = "THAT";
			break;
		default:
			fprintf(stderr, "Invalid index '%s'; line %i\n", ln->tokens[2], ln->truen);
			exit(1);
	}
	int newsz = sizeof(char) * 6;
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%s", ptr);
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
	checkasmsize(t, instcount);

	// instruction comment
	insts[0] = mkcom(t, ln);

	for(int i = 0; i < instcount; i++) {
		t->asmlns[t->asmind] = mkasmln(ln, insts[i]);
		t->asmind++;
	}
}

void startpoppush(struct Translator* t, struct line* ln, int indlen, char** insts) {
	// @segment
	insts[1] = switchseg(t, ln);
	
	// D=M
	insts[2] = heapstr(t, "D=M");

	// @i
	insts[3] = mkind(t, ln, indlen);
}

void pushcons(struct Translator* t, struct line* ln, int indlen) {
	// @i
	tpushcons[1] = mkind(t, ln, indlen);

	addasmlns(t, ln, tpushcons, TPUSHCONSN);
}

void pushstat(struct Translator* t, struct line* ln, int indlen) {
	// @fname.i
	tpushstat[1] = mkstatind(t, ln, indlen);

	addasmlns(t, ln, tpushstat, TPUSHSTATN);
}

void pushtemp(struct Translator* t, struct line* ln, int indlen) {
	// @5+i
	tpushtemp[1] = mktempind(t, ln, indlen);

	addasmlns(t, ln, tpushtemp, TPUSHTEMPN);
}

void pushpointer(struct Translator* t, struct line* ln, int indlen) {
	// @THIS/@THAT
	tpushpointer[1] = mkpointerind(t, ln, indlen);

	addasmlns(t, ln, tpushpointer, TPUSHPOINTERN);
}

void push(struct Translator* t, struct line* ln, int indlen) {
	startpoppush(t, ln, indlen, tpush);

	addasmlns(t, ln, tpush, TPUSHN);
}

void popstat(struct Translator* t, struct line* ln, int indlen) {
	// @fname.i
	tpopstat[TPOPSTATN-2] = mkstatind(t, ln, indlen);

	// M=D
	tpopstat[TPOPSTATN-1] = heapstr(t, "M=D");

	addasmlns(t, ln, tpopstat, TPOPSTATN);
}

void poptemp(struct Translator* t, struct line* ln, int indlen) {
	// @5+i
	tpoptemp[TPOPTEMPN-2] = mktempind(t, ln, indlen);

	// M=D
	tpoptemp[TPOPTEMPN-1] = heapstr(t, "M=D");

	addasmlns(t, ln, tpoptemp, TPOPTEMPN);
}

void poppointer(struct Translator* t, struct line* ln, int indlen) {
	// @THIS/@THAT
	tpoppointer[TPOPPOINTERN-2] = mkpointerind(t, ln, indlen);

	// M=D
	tpoppointer[TPOPPOINTERN-1] = heapstr(t, "M=D");

	addasmlns(t, ln, tpoppointer, TPOPPOINTERN);
}

void pop(struct Translator* t, struct line* ln, int indlen) {
	startpoppush(t, ln, indlen, tpop);

	addasmlns(t, ln, tpop, TPOPN);
}

void arith(struct Translator* t, struct line* ln, char* op) {
	tarith[TARITHN-1] = heapstr(t, op);

	addasmlns(t, ln, tarith, TARITHN);
}

void comp(struct Translator* t, struct line* ln, char* op) {
	char* label = mkcmplab(t, ln);
	int labellen = strlen(label);

	// @label
	tcomp[TCOMPN-6] = atlab(t, label, labellen);
	
	// D;J(op)
	int opsz = sizeof(char)*6;
	char* trueop = (char*)malloc(opsz);
	snprintf(trueop, opsz, "D;J%s", op);
	tcomp[TCOMPN-5] = trueop;
	pushtoclean(t, trueop);

	// (label)
	tcomp[TCOMPN-1] = mklab(t, label, labellen);

	addasmlns(t, ln, tcomp, TCOMPN);
};

void label(struct Translator* t, struct line* ln) {
	if(ln->tokenscount < 2) {
		fprintf(stderr, "Expected label; line %i", ln->truen);
		exit(1);
	}
	
	// (label)
	tlabel[TLABELN-1] = mklab(t, ln->tokens[1], strlen(ln->tokens[1]));

	addasmlns(t, ln, tlabel, TLABELN);
}

void mygoto(struct Translator* t, struct line* ln) {
	if(ln->tokenscount < 2) {
		fprintf(stderr, "Expected label; line %i", ln->truen);
		exit(1);
	}

	// @label
	tgoto[TGOTON-2] = atlab(t, ln->tokens[1], strlen(ln->tokens[1]));

	addasmlns(t, ln, tgoto, TGOTON);
}

void ifgoto(struct Translator* t, struct line* ln) {
	if(ln->tokenscount < 2) {
		fprintf(stderr, "Expected label; line %i", ln->truen);
		exit(1);
	}
	
	// @label
	tifgoto[TIFGOTON-2] = atlab(t, ln->tokens[1], strlen(ln->tokens[1]));

	addasmlns(t, ln, tifgoto, TIFGOTON);
}

void switchpush(struct Translator* t, struct line* ln) {
	checkopamnt(3, ln);
	char* seg = ln->tokens[1];
	int indlen = strlen(ln->tokens[2]);

	if(!strcmp(seg, "constant"))
		pushcons(t, ln, indlen);
	else if(!strcmp(seg, "static"))
		pushstat(t, ln, indlen);
	else if(!strcmp(seg, "temp"))
		pushtemp(t, ln, indlen);
	else if(!strcmp(seg, "pointer"))
		pushpointer(t, ln, indlen);
	else
		push(t, ln, indlen);
}

void switchpop(struct Translator* t, struct line* ln) {
	checkopamnt(3, ln);
	char* seg = ln->tokens[1];
	int indlen = strlen(ln->tokens[2]);

	if(!strcmp(seg, "static"))
		popstat(t, ln, indlen);
	else if(!strcmp(seg, "temp"))
		poptemp(t, ln, indlen);
	else if(!strcmp(seg, "pointer"))
		poppointer(t, ln, indlen);
	else
		pop(t, ln, indlen);
}

void switchop(struct Translator* t, struct line* ln) {
	char* op = ln->tokens[0];

	if(!strcmp(op, "push"))
		switchpush(t, ln);
	else if(!strcmp(op, "pop"))
		switchpop(t, ln);
	else if(!strcmp(op, "add"))
		arith(t, ln, "M=D+M");
	else if(!strcmp(op, "sub"))
		arith(t, ln, "M=M-D");
	else if(!strcmp(op, "neg"))
		addasmlns(t, ln, tneg, TNEGN);
	else if(!strcmp(op, "eq"))
		comp(t, ln, "EQ");
	else if(!strcmp(op, "gt"))
		comp(t, ln, "LT");
	else if(!strcmp(op, "lt"))
		comp(t, ln, "GT");
	else if(!strcmp(op, "and"))
		arith(t, ln, "M=D&M");
	else if(!strcmp(op, "or"))
		arith(t, ln, "M=D|M");
	else if(!strcmp(op, "not"))
		addasmlns(t, ln, tnot, TNOTN);
	else if(!strcmp(op, "label"))
		label(t, ln);
	else if(!strcmp(op, "goto"))
		mygoto(t, ln);
	else if(!strcmp(op, "if-goto"))
		ifgoto(t, ln);
	else {
		fprintf(stderr, "Unrecognized operation '%s'; line %i\n", op, ln->truen);
		exit(1);
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
	t->compcount = 0;
	t->asmlns = (struct asmln**)malloc(t->asmsize);

	t->tocleanind = 0;
	t->tocleansize = sizeof(char*)*(lns->count * 5);
	t->toclean = (char**)malloc(t->tocleansize);

	t->lns = lns;
	t->fname = fname;
	t->fnamelen = strlen(fname);

	return t;
}
