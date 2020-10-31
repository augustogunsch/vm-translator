#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "translator.h"

enum operation { push, pop };

enum operation getop(struct line* ln) {
	char* tok = ln->tokens[0];
	if(strcmp(tok, "push") == 0)
		return push;
	if(strcmp(tok, "pop") == 0)
		return pop;
	fprintf(stderr, "Invalid operation '%s'; line %i", tok, ln->truen);
	exit(1);
}

void freeasmlns(struct asmln** lns, int count) {
	for(int i = 0; i < count; i++) {
		free(lns[i]->instr);
		free(lns[i]);
	}
	free(lns);
}

char* heapstr(const char* input) {
	char* newstr = (char*)malloc(sizeof(char)*(strlen(input)+1));
	strcpy(newstr, input);
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
}

// produce comment as follows:
// pop/push segment i
char* mkcom(struct line* ln, int indlen) {
	int comlen = sizeof(char) * (strlen(ln->tokens[0]) + strlen(ln->tokens[1]) + indlen + 6);
	char* comment = (char*)malloc(comlen);
	snprintf(comment, comlen, "// %s %s %s", ln->tokens[0], ln->tokens[1], ln->tokens[2]);
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

char* mkind(char* ind, int indsz) {
	int newsz = sizeof(char) * (indsz + 2);
	char* newind = (char*)malloc(newsz);
	snprintf(newind, newsz, "@%s", ind);
	return newind;
}

void checkasmsize(int* size, struct asmln*** lns, int targ) {
	if(targ >= (*size)) {
		(*size)=targ*2;
		(*lns) = (struct asmln**)realloc((*lns), (*size));
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

void startpoppush(struct line* ln, struct asmln*** asmlns, int* asmind) {
	checkopamnt(3, ln);
	int asmi = *asmind;

	// // operation segment i
	int indlen = strlen(ln->tokens[2]);
	(*asmlns)[asmi] = mkasmln(ln, mkcom(ln, indlen));
	asmi++;
	
	// @segment
	(*asmlns)[asmi] = mkasmln(ln, switchseg(ln));
	asmi++;
	
	// D=M
	(*asmlns)[asmi] = mkasmln(ln, heapstr("D=M"));
	asmi++;

	// @i
	checkind(ln, indlen);
	(*asmlns)[asmi] = mkasmln(ln, mkind(ln->tokens[2], indlen));
	asmi++;

	(*asmind) = asmi;
}

void mkpush(struct line* ln, struct asmln*** asmlns, int* asmind, int* asmsize) {
	int asmi = *asmind;
	int totalinstrs = 11;
	checkasmsize(asmsize, asmlns, sizeof(struct asmln*)*(asmi+totalinstrs));

	startpoppush(ln, asmlns, &asmi);

	const int instcount = 7;
	const char* instrs[] = {
		"A=D+A",
		"D=M",
		"@SP",
		"A=M",
		"M=D",
		"@SP",
		"M=M+1"
	};

	for(int i = 0; i < instcount; i++) {
		(*asmlns)[asmi] = mkasmln(ln, heapstr(instrs[i]));
		asmi++;
	}

	(*asmind) = asmi;
}

void mkpop(struct line* ln, struct asmln*** asmlns, int* asmind, int* asmsize) {
	int asmi = *asmind;
	int finalasmi = 14;
	checkasmsize(asmsize, asmlns, sizeof(struct asmln*)*(asmi+finalasmi));

	startpoppush(ln, asmlns, &asmi);

	const int instcount = 10;
	const char* instrs[] = {
		"D=D+A",
		"@R13",
		"M=D",
		"@SP",
		"M=M-1",
		"A=M",
		"D=M",
		"@R13",
		"A=M",
		"M=D"
	};

	for(int i = 0; i < instcount; i++) {
		(*asmlns)[asmi] = mkasmln(ln, heapstr(instrs[i]));
		asmi++;
	}

	(*asmind) = asmi;
}

void switchop(struct line* ln, struct asmln*** asmlns, int* asmind, int* asmsize, enum operation op) {
	if(op == push)
		mkpush(ln, asmlns, asmind, asmsize);
	else if(op == pop)
		mkpop(ln, asmlns, asmind, asmsize);
}

struct asmln** translate(struct line** lns, int lnscount, int* asmcount) {
	int sizebet = sizeof(struct asmln*)*(lnscount * 15);
	int asmi = (*asmcount);
	struct asmln** asmlns = (struct asmln**)malloc(sizebet);
	
	for(int i = 0; i < lnscount; i++) {
		struct line* ln = lns[i];
		enum operation op = getop(ln);
		switchop(ln, &asmlns, &asmi, &sizebet, op);
	}

	(*asmcount) = asmi;
	return asmlns;
}
