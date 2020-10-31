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
	int asmcount = 0;
	struct asmln** asmlns = translate(lns, lncount, &asmcount);

	// freeing lns
	freelns(lns, lncount);
	
	// printing
	for(int i = 0; i < asmcount; i++) {
		printf("%s\n", asmlns[i]->instr);
	}

	// freeing asmlns
	freeasmlns(asmlns, asmcount);

	return 0;
}
