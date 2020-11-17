#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "translator.h"
#include "templates.h"
#include "util.h"

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
}

void freetranslator(struct Translator* t) {
	free(t->asmlns);
	freetoclean(t);
	free(t);
}

void printasmlns(struct Translator* t, FILE* stream) {
	for(int i = 0; i < t->asmind; i++)
		fprintf(stream, "%s\n", t->asmlns[i]);
}

char* heapstrtoclean(struct Translator* t, const char* input) {
	char* newstr = heapstr(input, strlen(input));
	pushtoclean(t, newstr);
	return newstr;
}

char* switchseg(struct Translator* t, struct line* ln) {
	char* seg = ln->tokens[1];
	if(!strcmp(seg, "local"))
		return heapstrtoclean(t, "@LCL");
	if(!strcmp(seg, "argument"))
		return heapstrtoclean(t, "@ARG");
	if(!strcmp(seg, "this"))
		return heapstrtoclean(t, "@THIS");
	if(!strcmp(seg, "that"))
		return heapstrtoclean(t, "@THAT");
	fprintf(stderr, "Unrecognized segment '%s'; file %s.vm, line %i\n", t->fname, seg, ln->truen);
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

void checknumber(struct Translator* t, struct line* ln, int indlen, char* name, char* n) {
	for(int i = 0; i < indlen; i++)
		if(!isdigit(n[i])) {
			fprintf(stderr, "Invalid %s '%s'; file %s.vm, line %i\n", t->fname, name, n, ln->truen);
			exit(1);
		}
}

void checknargs(struct Translator* t, struct line* ln, int nargslen) {
	checknumber(t, ln, nargslen, "argument number", ln->tokens[2]);
}

void checknlocals(struct Translator* t, struct line* ln, int nlocalslen) {
	checknumber(t, ln, nlocalslen, "local variable number", ln->tokens[2]);
}

void checkind(struct Translator* t, struct line* ln, int indlen) {
	checknumber(t, ln, indlen, "index", ln->tokens[2]);
}

int countplaces(int n) {
	int places = 1;
	int divisor = 1;
	if(n < 0) {
		n = -n;
		places++;
	}
	while(n / divisor >= 10) {
		places++;
		divisor *= 10;
	}
	return places;
}

void checkinfun(struct Translator* t, struct line* ln) {
	if(t->funcount <= 0) {
		fprintf(stderr, "Instruction should be part of a function; file %s.vm, line %i\n", t->fname, ln->truen);
		exit(1);
	}
}

char* mkspeciallab(struct Translator* t, struct line* ln, char* suffix, int* ind) {
	checkinfun(t, ln);
	(*ind)++;
	int sz = (t->lastfunlen + countplaces(*ind) + strlen(suffix) + 3) * sizeof(char);
	char* lab = (char*)malloc(sz);
	snprintf(lab, sz, "%s$%s.%i", t->lastfun, suffix, (*ind));
	pushtoclean(t, lab);
	return lab;
}

char* mkcmplab(struct Translator* t, struct line* ln) {
	return mkspeciallab(t, ln, "cmp", &(t->cmpind));
}

char* mkretlab(struct Translator* t, struct line* ln) {
	return mkspeciallab(t, ln, "ret", &(t->retind));
}

char* mkind(struct Translator* t, struct line* ln, int indlen) {
	checkind(t, ln, indlen);
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

char* atn(struct Translator* t, int n) {
	int newsz = sizeof(char) * (countplaces(n) + 2);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%i", n);
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

char* mkgotolab(struct Translator* t, struct line* ln) {
	int sz = sizeof(char) * (t->lastfunlen + strlen(ln->tokens[1]) + 3);
	char* lab = (char*)malloc(sz);
	snprintf(lab, sz, "@%s$%s", t->lastfun, ln->tokens[1]);
	pushtoclean(t, lab);
	return lab;
}

char* mkstatind(struct Translator* t, struct line* ln, int indlen) {
	checkind(t, ln, indlen);
	int newsz = sizeof(char) * (t->fnamelen + indlen + 3);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%s.%s", t->fname, ln->tokens[2]);
	pushtoclean(t, newind);
	return newind;
}

char* mktempind(struct Translator* t, struct line* ln, int indlen) {
	checkind(t, ln, indlen);
	int intind = atoi(ln->tokens[2]);
	int newsz = sizeof(char) * (indlen + 3);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%i", intind+5);
	pushtoclean(t, newind);
	return newind;
}

char* mkpointerind(struct Translator* t, struct line* ln, int indlen) {
	if(indlen > 1) {
		fprintf(stderr, "Invalid index '%s'; file %s.vm, line %i\n", t->fname, ln->tokens[2], ln->truen);
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
			fprintf(stderr, "Invalid index '%s'; file %s.vm, line %i\n", t->fname, ln->tokens[2], ln->truen);
			exit(1);
	}
	int newsz = sizeof(char) * 6;
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%s", ptr);
	pushtoclean(t, newind);
	return newind;
}

void checkasmsize(struct Translator* t, int toadd) {
	int targ = sizeof(char*)*(t->asmind+toadd);
	if(targ >= t->asmsize) {
		t->asmsize = targ * 2;
		t->asmlns = (char**)realloc(t->asmlns, t->asmsize);
	}
}

void checkopamnt(struct Translator* t, int amnt, struct line* ln) {
	if(ln->tokenscount < 2) {
		fprintf(stderr, "Missing memory segment; file %s.vm, line %i\n", t->fname, ln->truen);
		exit(1);
	}
	if(amnt > 2)
		if(ln->tokenscount < 3) {
			fprintf(stderr, "Missing operation index; file %s.vm, line %i\n", t->fname, ln->truen);
			exit(1);
		}
}

void checklab(struct Translator* t, struct line* ln) {
	if(ln->tokenscount < 2) {
		fprintf(stderr, "Expected label; file %s.vm, line %i\n", t->fname, ln->truen);
		exit(1);
	}
}

void checkfun(struct Translator* t, struct line* ln) {
	if(ln->tokenscount < 2) {
		fprintf(stderr, "Expected function; file %s.vm, line %i\n", t->fname, ln->truen);
		exit(1);
	}

	if(ln->tokenscount < 3) {
		fprintf(stderr, "Expected argument amount; file %s.vm, line %i\n", t->fname, ln->truen);
		exit(1);
	}
}

void addasm(struct Translator* t, char** insts, int instcount) {
	checkasmsize(t, instcount);

	for(int i = 0; i < instcount; i++) {
		t->asmlns[t->asmind] = insts[i];
		t->asmind++;
	}
}

void addasmlns(struct Translator* t, struct line* ln, char** insts, int instcount) {
	// instruction comment
	insts[0] = mkcom(t, ln);
	
	addasm(t, insts, instcount);
}

void startpoppush(struct Translator* t, struct line* ln, int indlen, char** insts) {
	// @segment
	insts[1] = switchseg(t, ln);
	
	// D=M
	insts[2] = heapstrtoclean(t, "D=M");

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
	tpopstat[TPOPSTATN-1] = heapstrtoclean(t, "M=D");

	addasmlns(t, ln, tpopstat, TPOPSTATN);
}

void poptemp(struct Translator* t, struct line* ln, int indlen) {
	// @5+i
	tpoptemp[TPOPTEMPN-2] = mktempind(t, ln, indlen);

	// M=D
	tpoptemp[TPOPTEMPN-1] = heapstrtoclean(t, "M=D");

	addasmlns(t, ln, tpoptemp, TPOPTEMPN);
}

void poppointer(struct Translator* t, struct line* ln, int indlen) {
	// @THIS/@THAT
	tpoppointer[TPOPPOINTERN-2] = mkpointerind(t, ln, indlen);

	// M=D
	tpoppointer[TPOPPOINTERN-1] = heapstrtoclean(t, "M=D");

	addasmlns(t, ln, tpoppointer, TPOPPOINTERN);
}

void pop(struct Translator* t, struct line* ln, int indlen) {
	startpoppush(t, ln, indlen, tpop);

	addasmlns(t, ln, tpop, TPOPN);
}

void arith(struct Translator* t, struct line* ln, char* op) {
	tarith[TARITHN-1] = heapstrtoclean(t, op);

	addasmlns(t, ln, tarith, TARITHN);
}

void comp(struct Translator* t, struct line* ln, char* op) {
	char* label = mkcmplab(t, ln);
	int labellen = strlen(label);

	// @label
	tcomp[TCOMPN-6] = atlab(t, label, labellen);
	
	// D;J(op)
	int opsz = sizeof(char) * 6;
	char* trueop = (char*)malloc(opsz);
	snprintf(trueop, opsz, "D;J%s", op);
	tcomp[TCOMPN-5] = trueop;
	pushtoclean(t, trueop);

	// (label)
	tcomp[TCOMPN-1] = mklab(t, label, labellen);

	addasmlns(t, ln, tcomp, TCOMPN);
};

void label(struct Translator* t, struct line* ln) {
	checklab(t, ln);
	
	// (funcname$label)
	checkinfun(t, ln);
	int sz = (t->lastfunlen + strlen(ln->tokens[1]) + 4) * sizeof(char);
	char* lab = (char*)malloc(sz);
	snprintf(lab, sz, "(%s$%s)", t->lastfun, ln->tokens[1]);
	pushtoclean(t, lab);
	tlabel[TLABELN-1] = lab;

	addasmlns(t, ln, tlabel, TLABELN);
}

void mygoto(struct Translator* t, struct line* ln) {
	checklab(t, ln);

	// @label
	tgoto[TGOTON-2] = mkgotolab(t, ln);

	addasmlns(t, ln, tgoto, TGOTON);
}

void ifgoto(struct Translator* t, struct line* ln) {
	checklab(t, ln);
	
	// @label
	tifgoto[TIFGOTON-2] = mkgotolab(t, ln);

	addasmlns(t, ln, tifgoto, TIFGOTON);
}

int pushframe(struct Translator* t, struct line* ln, char* retlab, int retlablen) {
	tcallstart[1] = atlab(t, retlab, retlablen);

	addasmlns(t, ln, tcallstart, TCALLSTARTN);

	for(int i = 0; i < TFRAMEVARSN; i++) {
		tcallpush[0] = tframevars[i];
		addasm(t, tcallpush, TCALLPUSHN);
	}

	return TFRAMEVARSN + 1;
}

void call(struct Translator* t, struct line* ln) {
	checkfun(t, ln);
	
	// return label
	char* retlab = mkretlab(t, ln);
	int retlablen = strlen(retlab);

	// push frame
	int framesize = pushframe(t, ln, retlab, retlablen);

	// setting ARG
	int nargslen = strlen(ln->tokens[2]);
	checknargs(t, ln, nargslen);
	int nargs = atoi(ln->tokens[2]);
	tcallsetarg[TCALLSETARGN-4] = atn(t, nargs + framesize);
	addasm(t, tcallsetarg, TCALLSETARGN);
	
	// jmp
	int jmplen = strlen(ln->tokens[1]);
	tcalljmp[TCALLJMPN-3] = atlab(t, ln->tokens[1], jmplen);
	tcalljmp[TCALLJMPN-1] = mklab(t, retlab, retlablen);
	addasm(t, tcalljmp, TCALLJMPN);
}

void function(struct Translator* t, struct line* ln) {
	if(!t->returned) {
		fprintf(stderr, "Last function did not return; file %s.vm, line %i\n", t->fname, ln->truen);
		exit(1);
	}

	checkfun(t, ln);

	t->lastfun = ln->tokens[1];
	int funlen = strlen(ln->tokens[1]);
	t->lastfunlen = funlen;
	t->funcount++;
	t->retind = 0;
	t->cmpind = 0;

	// (funcname)
	tfunction[1] = mklab(t, ln->tokens[1], funlen);
	addasmlns(t, ln, tfunction, TFUNCTIONN);

	// repeat nVars times:
	int nlocalslen = strlen(ln->tokens[2]);
	checknlocals(t, ln, nlocalslen);
	int nlocals = atoi(ln->tokens[2]);

	for(int i = 0; i < nlocals; i++) {
		addasm(t, tfunctionpush, TFUNCTIONPUSHN);
	}
}

void myreturn(struct Translator* t, struct line* ln) {
	addasmlns(t, ln, tstartreturn, TSTARTRETURNN);
	
	for(int i = TFRAMEVARSN-1; i >= 0; i--) {
		tretpop[TRETPOPN-2] = tframevars[i];
		addasm(t, tretpop, TRETPOPN);
	}

	addasm(t, tendreturn, TENDRETURNN);
}

void switchpush(struct Translator* t, struct line* ln) {
	checkopamnt(t, 3, ln);
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
	checkopamnt(t, 3, ln);
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
	short returned = 0;

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
	else if(!strcmp(op, "return")) {
		myreturn(t, ln);
		returned = 1;
	}
	else if(!strcmp(op, "function"))
		function(t, ln);
	else if(!strcmp(op, "call"))
		call(t, ln);
	else {
		fprintf(stderr, "Unrecognized operation '%s'; file %s.vm, line %i\n", t->fname, op, ln->truen);
		exit(1);
	}

	t->returned = returned;
}

void translate(struct Translator* t) {
	for(int i = 0; i < t->lns->count; i++)
		switchop(t, t->lns->lns[i]);

	if(!t->returned) {
		fprintf(stderr, "Expected return before end of file; file %s.vm, line %i\n", t->fname, t->lns->count-1);
		exit(1);
	}
}

struct Translator* mktranslator(struct lnarray* lns, char* fname) {
	struct Translator* t = (struct Translator*)malloc(sizeof(struct Translator));
	t->asmsize = sizeof(char*)*(lns->count * 15);
	t->asmind = 0;
	t->funcount = 0;
	t->retind = 0;
	t->cmpind = 0;
	t->returned = 1;
	t->asmlns = (char**)malloc(t->asmsize);

	t->tocleanind = 0;
	t->tocleansize = sizeof(char*)*(lns->count * 5);
	t->toclean = (char**)malloc(t->tocleansize);

	t->lns = lns;
	t->fname = fname;
	t->fnamelen = strlen(fname);

	return t;
}
