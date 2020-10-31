#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "parser.h"
#include "translator.h"

char* verfname(char* fname) {
	int len = strlen(fname);
	if(len > 3) {
		int extind = len - 3;
		char* extstr = fname + (sizeof(char)*extind);
		if(strcmp(extstr, ".vm") == 0) {
			int startind = 0;
			for(int i = extind-1; i >= 0; i--) {
				if(fname[i] == '/') {
					startind = i+1;
					break;
				}
			}
			int size = sizeof(char)*(extind - startind);
			char* startstr = fname + (sizeof(char)*startind);
			char* retstr = (char*)malloc(size);
			strncpy(retstr, startstr, size);
			return retstr;
		}
	}
	fprintf(stderr, "Name format must be Xxx.vm\n");
	exit(1);
}

int main(int argc, char* argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Usage: %s {file}\n", argv[0]);
		return 1;
	}
	// file name validating
	char* fname = verfname(argv[1]);
	printf("%s\n", fname);
	
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
