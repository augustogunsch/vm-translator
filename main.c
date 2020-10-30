#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "parser.h"

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

	int lncount, widestln, maxtokens;
	getinfo(input, &lncount, &widestln, &maxtokens);
	struct line** lns = parse(input, lncount, widestln, maxtokens);
	
	printf("lns: %i\n", lncount);
	for(int i = 0; i < lncount; i++) {
		int tkcount = lns[i]->tokenscount;
		for(int j = 0; j < tkcount; j++) {
			printf("%s ", lns[i]->tokens[j]);
		}
		printf("\n");
	}

	return 0;
}
