#include <string.h>
#include <stdlib.h>
#include "util.h"

char* heapstr(const char* str, int len) {
	int sz = sizeof(char) * (len + 1);
	char* outstr = (char*)malloc(sz);
	strcpy(outstr, str);
	return outstr;
}
