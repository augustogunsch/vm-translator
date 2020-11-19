#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "translator.h"
#include "templates.h"
#include "util.h"

void pushtoclean(TRANSLATOR* t, char* topush) {
	int nextsz = sizeof(char*)*(t->toclean->count+1);
	if(nextsz >= t->toclean->size) {
		t->toclean->size = nextsz * 2;
		t->toclean->items = realloc(t->toclean->items, t->toclean->size);
	}
	t->toclean->items[t->toclean->count] = topush;
	t->toclean->count++;
}

void freetoclean(TRANSLATOR* t) {
	for(int i = 0; i < t->toclean->count; i++)
		free(t->toclean->items[i]);
	free(t->toclean->items);
	free(t->toclean);
}

void freetranslator(TRANSLATOR* t) {
	free(t->asmlns->items);
	free(t->asmlns);
	freetoclean(t);
	free(t);
}

void printasmlns(TRANSLATOR* t, FILE* stream) {
	for(int i = 0; i < t->asmlns->count; i++)
		fprintf(stream, "%s\n", t->asmlns->items[i]);
}

char* heapstrtoclean(TRANSLATOR* t, const char* input) {
	char* newstr = heapstr(input, strlen(input));
	pushtoclean(t, newstr);
	return newstr;
}

char* switchseg(TRANSLATOR* t, LINE* ln) {
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

int lnlen(int* out, LINE* ln) {
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
char* mkcom(TRANSLATOR* t, LINE* ln) {
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

void checknumber(TRANSLATOR* t, LINE* ln, int indlen, char* name, char* n) {
	for(int i = 0; i < indlen; i++)
		if(!isdigit(n[i])) {
			fprintf(stderr, "Invalid %s '%s'; file %s.vm, line %i\n", t->fname, name, n, ln->truen);
			exit(1);
		}
}

void checknargs(TRANSLATOR* t, LINE* ln, int nargslen) {
	checknumber(t, ln, nargslen, "argument number", ln->tokens[2]);
}

void checknlocals(TRANSLATOR* t, LINE* ln, int nlocalslen) {
	checknumber(t, ln, nlocalslen, "local variable number", ln->tokens[2]);
}

void checkind(TRANSLATOR* t, LINE* ln, int indlen) {
	checknumber(t, ln, indlen, "index", ln->tokens[2]);
}

void checkinfun(TRANSLATOR* t, LINE* ln) {
	if(t->funcount <= 0) {
		fprintf(stderr, "Instruction should be part of a function; file %s.vm, line %i\n", t->fname, ln->truen);
		exit(1);
	}
}

char* mkspeciallab(TRANSLATOR* t, LINE* ln, char* suffix, int* ind) {
	checkinfun(t, ln);
	(*ind)++;
	int sz = (t->lastfunlen + countplaces(*ind) + strlen(suffix) + 3) * sizeof(char);
	char* lab = (char*)malloc(sz);
	snprintf(lab, sz, "%s$%s.%i", t->lastfun, suffix, (*ind));
	pushtoclean(t, lab);
	return lab;
}

char* mkcmplab(TRANSLATOR* t, LINE* ln) {
	return mkspeciallab(t, ln, "cmp", &(t->cmpind));
}

char* mkretlab(TRANSLATOR* t, LINE* ln) {
	return mkspeciallab(t, ln, "ret", &(t->retind));
}

char* mkind(TRANSLATOR* t, LINE* ln, int indlen) {
	checkind(t, ln, indlen);
	int newsz = sizeof(char) * (indlen + 2);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%s", ln->tokens[2]);
	pushtoclean(t, newind);
	return newind;
}

char* atlab(TRANSLATOR* t, char* label, int labellen) {
	int newsz = sizeof(char) * (labellen + 2);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%s", label);
	pushtoclean(t, newind);
	return newind;
}

char* atn(TRANSLATOR* t, int n) {
	int newsz = sizeof(char) * (countplaces(n) + 2);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%i", n);
	pushtoclean(t, newind);
	return newind;
}

char* mklab(TRANSLATOR* t, char* label, int labellen) {
	int newsz = sizeof(char) * (labellen + 3);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "(%s)", label);
	pushtoclean(t, newind);
	return newind;
}

char* mkgotolab(TRANSLATOR* t, LINE* ln) {
	int sz = sizeof(char) * (t->lastfunlen + strlen(ln->tokens[1]) + 3);
	char* lab = (char*)malloc(sz);
	snprintf(lab, sz, "@%s$%s", t->lastfun, ln->tokens[1]);
	pushtoclean(t, lab);
	return lab;
}

char* mkstatind(TRANSLATOR* t, LINE* ln, int indlen) {
	checkind(t, ln, indlen);
	int newsz = sizeof(char) * (t->fnamelen + indlen + 3);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%s.%s", t->fname, ln->tokens[2]);
	pushtoclean(t, newind);
	return newind;
}

char* mktempind(TRANSLATOR* t, LINE* ln, int indlen) {
	checkind(t, ln, indlen);
	int intind = atoi(ln->tokens[2]);
	int newsz = sizeof(char) * (indlen + 3);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%i", intind+5);
	pushtoclean(t, newind);
	return newind;
}

char* mkpointerind(TRANSLATOR* t, LINE* ln, int indlen) {
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

void checkasmsize(TRANSLATOR* t, int toadd) {
	int targ = sizeof(char*)*(t->asmlns->count+toadd);
	if(targ >= t->asmlns->size) {
		t->asmlns->size = targ * 2;
		t->asmlns->items = (char**)realloc(t->asmlns->items, t->asmlns->size);
	}
}

void checkopamnt(TRANSLATOR* t, int amnt, LINE* ln) {
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

void checklab(TRANSLATOR* t, LINE* ln) {
	if(ln->tokenscount < 2) {
		fprintf(stderr, "Expected label; file %s.vm, line %i\n", t->fname, ln->truen);
		exit(1);
	}
}

void checkfun(TRANSLATOR* t, LINE* ln) {
	if(ln->tokenscount < 2) {
		fprintf(stderr, "Expected function; file %s.vm, line %i\n", t->fname, ln->truen);
		exit(1);
	}

	if(ln->tokenscount < 3) {
		fprintf(stderr, "Expected argument amount; file %s.vm, line %i\n", t->fname, ln->truen);
		exit(1);
	}
}

void addasm(TRANSLATOR* t, TEMPLATE* tp) {
	checkasmsize(t, tp->count);

	for(int i = 0; i < tp->count; i++) {
		t->asmlns->items[t->asmlns->count] = tp->items[i];
		t->asmlns->count++;
	}
}

void addasmlns(TRANSLATOR* t, LINE* ln, TEMPLATE* tp) {
	// instruction comment
	tp->items[0] = mkcom(t, ln);
	
	addasm(t, tp);
}

void startpoppush(TRANSLATOR* t, LINE* ln, int indlen, TEMPLATE* tp) {
	// @segment
	tp->items[1] = switchseg(t, ln);
	
	// D=M
	tp->items[2] = heapstrtoclean(t, "D=M");

	// @i
	tp->items[3] = mkind(t, ln, indlen);
}

void pushcons(TRANSLATOR* t, LINE* ln, int indlen) {
	// @i
	tpushcons.items[1] = mkind(t, ln, indlen);

	addasmlns(t, ln, &tpushcons);
}

void pushstat(TRANSLATOR* t, LINE* ln, int indlen) {
	// @fname.i
	tpushstat.items[1] = mkstatind(t, ln, indlen);

	addasmlns(t, ln, &tpushstat);
}

void pushtemp(TRANSLATOR* t, LINE* ln, int indlen) {
	// @5+i
	tpushtemp.items[1] = mktempind(t, ln, indlen);

	addasmlns(t, ln, &tpushtemp);
}

void pushpointer(TRANSLATOR* t, LINE* ln, int indlen) {
	// @THIS/@THAT
	tpushpointer.items[1] = mkpointerind(t, ln, indlen);

	addasmlns(t, ln, &tpushpointer);
}

void push(TRANSLATOR* t, LINE* ln, int indlen) {
	startpoppush(t, ln, indlen, &tpush);

	addasmlns(t, ln, &tpush);
}

void popstat(TRANSLATOR* t, LINE* ln, int indlen) {
	// @fname.i
	tpopstat.items[tpopstat.count-2] = mkstatind(t, ln, indlen);

	// M=D
	tpopstat.items[tpopstat.count-1] = heapstrtoclean(t, "M=D");

	addasmlns(t, ln, &tpopstat);
}

void poptemp(TRANSLATOR* t, LINE* ln, int indlen) {
	// @5+i
	tpoptemp.items[tpoptemp.count-2] = mktempind(t, ln, indlen);

	// M=D
	tpoptemp.items[tpoptemp.count-1] = heapstrtoclean(t, "M=D");

	addasmlns(t, ln, &tpoptemp);
}

void poppointer(TRANSLATOR* t, LINE* ln, int indlen) {
	// @THIS/@THAT
	tpoppointer.items[tpoppointer.count-2] = mkpointerind(t, ln, indlen);

	// M=D
	tpoppointer.items[tpoppointer.count-1] = heapstrtoclean(t, "M=D");

	addasmlns(t, ln, &tpoppointer);
}

void pop(TRANSLATOR* t, LINE* ln, int indlen) {
	startpoppush(t, ln, indlen, &tpop);

	addasmlns(t, ln, &tpop);
}

void arith(TRANSLATOR* t, LINE* ln, char* op) {
	tarith.items[tarith.count-1] = heapstrtoclean(t, op);

	addasmlns(t, ln, &tarith);
}

void comp(TRANSLATOR* t, LINE* ln, char* op) {
	char* label = mkcmplab(t, ln);
	int labellen = strlen(label);

	// @label
	tcomp.items[tcomp.count-6] = atlab(t, label, labellen);
	
	// D;J(op)
	int opsz = sizeof(char) * 6;
	char* trueop = (char*)malloc(opsz);
	snprintf(trueop, opsz, "D;J%s", op);
	tcomp.items[tcomp.count-5] = trueop;
	pushtoclean(t, trueop);

	// (label)
	tcomp.items[tcomp.count-1] = mklab(t, label, labellen);

	addasmlns(t, ln, &tcomp);
};

void label(TRANSLATOR* t, LINE* ln) {
	checklab(t, ln);
	
	// (funcname$label)
	checkinfun(t, ln);
	int sz = (t->lastfunlen + strlen(ln->tokens[1]) + 4) * sizeof(char);
	char* lab = (char*)malloc(sz);
	snprintf(lab, sz, "(%s$%s)", t->lastfun, ln->tokens[1]);
	pushtoclean(t, lab);
	tlabel.items[tlabel.count-1] = lab;

	addasmlns(t, ln, &tlabel);
}

void mygoto(TRANSLATOR* t, LINE* ln) {
	checklab(t, ln);

	// @label
	tgoto.items[tgoto.count-2] = mkgotolab(t, ln);

	addasmlns(t, ln, &tgoto);
}

void ifgoto(TRANSLATOR* t, LINE* ln) {
	checklab(t, ln);
	
	// @label
	tifgoto.items[tifgoto.count-2] = mkgotolab(t, ln);

	addasmlns(t, ln, &tifgoto);
}

int pushframe(TRANSLATOR* t, LINE* ln, char* retlab, int retlablen) {
	tcallstart.items[1] = atlab(t, retlab, retlablen);

	addasmlns(t, ln, &tcallstart);

	for(int i = 0; i < tframevars.count; i++) {
		tcallpush.items[0] = tframevars.items[i];
		addasm(t, &tcallpush);
	}

	return tframevars.count + 1;
}

void call(TRANSLATOR* t, LINE* ln) {
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
	tcallsetarg.items[tcallsetarg.count-4] = atn(t, nargs + framesize);
	addasm(t, &tcallsetarg);
	
	// jmp
	int jmplen = strlen(ln->tokens[1]);
	tcalljmp.items[tcalljmp.count-3] = atlab(t, ln->tokens[1], jmplen);
	tcalljmp.items[tcalljmp.count-1] = mklab(t, retlab, retlablen);
	addasm(t, &tcalljmp);
}

void function(TRANSLATOR* t, LINE* ln) {
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
	tfunction.items[1] = mklab(t, ln->tokens[1], funlen);
	addasmlns(t, ln, &tfunction);

	// repeat nVars times:
	int nlocalslen = strlen(ln->tokens[2]);
	checknlocals(t, ln, nlocalslen);
	int nlocals = atoi(ln->tokens[2]);

	for(int i = 0; i < nlocals; i++) {
		addasm(t, &tfunctionpush);
	}
}

void myreturn(TRANSLATOR* t, LINE* ln) {
	addasmlns(t, ln, &tstartreturn);
	
	for(int i = tframevars.count-1; i >= 0; i--) {
		tretpop.items[tretpop.count-2] = tframevars.items[i];
		addasm(t, &tretpop);
	}

	addasm(t, &tendreturn);
}

void switchpush(TRANSLATOR* t, LINE* ln) {
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

void switchpop(TRANSLATOR* t, LINE* ln) {
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

void switchop(TRANSLATOR* t, LINE* ln) {
	char* op = ln->tokens[0];
	bool returned = false;

	if(!strcmp(op, "push"))
		switchpush(t, ln);
	else if(!strcmp(op, "pop"))
		switchpop(t, ln);
	else if(!strcmp(op, "add"))
		arith(t, ln, "M=D+M");
	else if(!strcmp(op, "sub"))
		arith(t, ln, "M=M-D");
	else if(!strcmp(op, "neg"))
		addasmlns(t, ln, &tneg);
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
		addasmlns(t, ln, &tnot);
	else if(!strcmp(op, "label"))
		label(t, ln);
	else if(!strcmp(op, "goto"))
		mygoto(t, ln);
	else if(!strcmp(op, "if-goto"))
		ifgoto(t, ln);
	else if(!strcmp(op, "return")) {
		myreturn(t, ln);
		returned = true;
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

void translate(TRANSLATOR* t) {
	for(int i = 0; i < t->lns->count; i++)
		switchop(t, t->lns->lns[i]);

	if(!t->returned) {
		fprintf(stderr, "Expected return before end of file; file %s.vm, line %i\n", t->fname, t->lns->count-1);
		exit(1);
	}
}

TRANSLATOR* mktranslator(LINEARRAY* lns, char* fname) {
	TRANSLATOR* t = (TRANSLATOR*)malloc(sizeof(TRANSLATOR));

	t->asmlns = (STRLIST*)malloc(sizeof(STRLIST));
	t->asmlns->count = 0;
	t->asmlns->size = sizeof(char*)*(lns->count * 15);
	t->asmlns->items = (char**)malloc(t->asmlns->size);

	t->toclean = (STRLIST*)malloc(sizeof(STRLIST));
	t->toclean->count = 0;
	t->toclean->size = sizeof(char*)*(lns->count * 5);
	t->toclean->items = (char**)malloc(t->toclean->size);

	t->funcount = 0;
	t->retind = 0;
	t->cmpind = 0;
	t->returned = true;

	t->lns = lns;
	t->fname = fname;
	t->fnamelen = strlen(fname);

	return t;
}
