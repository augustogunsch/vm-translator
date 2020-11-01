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
			int size = sizeof(char)*(extind - startind + 1);
			char* startstr = fname + (sizeof(char)*startind);
			char* retstr = (char*)malloc(size);
			snprintf(retstr, size, "%s", startstr);
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
	translate(lns, lncount, fname);

	// freeing lns
	freelns(lns, lncount);
	
	// printing
	printasmlns();

	// freeing asmlns
	free(fname);
	freeasmlns();

	return 0;
}
