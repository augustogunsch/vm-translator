#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "parser.h"
#include "translator.h"

int main(int argc, char* argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Usage: %s {file}\n", argv[0]);
		return 1;
	}
	
	FILE* input = fopen(argv[1], "r");

	if(input == NULL) {
		fprintf(stderr, "%s\n", strerror(errno));
		return errno;
	}

	// info gathering
	int lncount, widestln, maxtokens;
	getinfo(input, &lncount, &widestln, &maxtokens);

	// parsing
	struct line** lns = parse(input, lncount, widestln, maxtokens);
	fclose(input);

	// translating
	int asmind = 0;
	struct asmln** asmlns = translate(lns, lncount, &asmind);
	
	// printing
	for(int i = 0; i < asmind; i++) {
		printf("%s\n", asmlns[i]->instr);
	}
	/*
	for(int i = 0; i < lncount; i++) {
		int tkcount = lns[i]->tokenscount;
		for(int j = 0; j < tkcount; j++) {
			printf("%s ", lns[i]->tokens[j]);
		}
		printf("\n");
	}
	*/

	// freeing
	freelns(lns, lncount);

	return 0;
}
