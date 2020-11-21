#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>
#include <unistd.h>
#include "parser.h"
#include "translator.h"
#include "bootstrap.h"
#include "util.h"

#include <limits.h>
#ifndef PATH_MAX
#ifdef __linux__
#include <linux/limits.h>
#else
#define PATH_MAX 512
#endif
#endif

typedef struct {
	char** files;
	char** fnames;
	int filecount;
	int filessz;
	char* output;
} TRANSLATIONLIST;

char* getname(char* f, int len) {
	int startind = 0;
	int endind = len - 1;
	bool readsmt = false;
 
	for(int i = endind; i >= 0; i--) {
		if(f[i] == '/') {
			if(!readsmt) {
				endind = i-1;
				f[i] = '\0';
				continue;
			}
			startind = i+1;
			break;
		}
		if(f[i] == '.')
			endind = i-1;
		readsmt = true;
	}

	int size = sizeof(char)*(endind - startind + 2);
	char* startstr = f + (sizeof(char)*startind);
	char* retstr = (char*)malloc(size);
	snprintf(retstr, size, "%s", startstr);
	return retstr;
}

char* getfullname(char* f, int len) {
	int endind = len - 1;
	bool readsmt = false;
 
	for(int i = endind; i >= 0; i--) {
		if(f[i] == '/') {
			if(!readsmt) {
				endind = i-1;
				continue;
			}
			break;
		}
		if(f[i] == '.')
			endind = i-1;
		readsmt = true;
	}

	int size = sizeof(char)*(endind + 2);
	char* retstr = (char*)malloc(size);
	snprintf(retstr, size, "%s", f);
	return retstr;
}

bool isdotvm(char* f, int extind) {
	char* extstr = f + (sizeof(char) * extind);
	return strcmp(extstr, ".vm") == 0;
}

bool isdir(char* f, int len) {
	bool readsmt = false;
	for(int i = len-1; i >= 0; i--) {
		if(f[i] == '.')
			if(readsmt)
				return false;
			else
				continue;
		if(f[i] == '/')
			return 1;
		readsmt = true;
	}
	return true;
}

char* getoutname(char* input, int len, bool isdir) {
	char* outname;
	if(isdir) {
		char olddir[PATH_MAX];
		getcwd(olddir, PATH_MAX);

		chdir(input);
		char buf[PATH_MAX];
		getcwd(buf, PATH_MAX);

		chdir(olddir);

		char* name = getname(buf, strlen(buf));
		int sz = sizeof(char) * (strlen(name)+len+6);
		outname = (char*)malloc(sz);
		snprintf(outname, sz, "%s/%s.asm", input, name);
		free(name);
	}
	else {
		char* name = getfullname(input, len);
		int sz = sizeof(char) * (strlen(name)+5);
		outname = (char*)malloc(sz);
		snprintf(outname, sz, "%s.asm", name);
		free(name);
	}
	return outname;
}

void addfile(TRANSLATIONLIST* l, char* fullname, char* name) {
	int count = l->filecount;
	int targsize = (count + 1) * sizeof(char*);

	if(l->filessz < targsize) {
		int newsz = targsize * 2;
		l->files = realloc(l->files, newsz);
		l->fnames = realloc(l->fnames, newsz);
		l->filessz = newsz;
	}

	l->files[count] = fullname;
	l->fnames[count] = name;
	l->filecount++;
}

TRANSLATIONLIST* getfiles(char* input) {
	int filessz = sizeof(char*) * 16;
	TRANSLATIONLIST* filelist = (TRANSLATIONLIST*)malloc(sizeof(TRANSLATIONLIST));
	filelist->files = (char**)malloc(filessz);
	filelist->fnames = (char**)malloc(filessz);
	filelist->filessz = filessz;
	filelist->filecount = 0;

	int inplen = strlen(input);
	bool isitdir = isdir(input, inplen);
	if(isitdir) {
		DIR* dir = opendir(input);

		if(dir == NULL) {
			fprintf(stderr, "%s\n", strerror(errno));
			exit(1);
		}

		struct dirent* thisfile;
		while(thisfile = readdir(dir), thisfile != NULL) {
			int len = strlen(thisfile->d_name);
			if(len > 3) {
				int extind = len - 3;
				if(isdotvm(thisfile->d_name, extind)) {
					int sz = sizeof(char)*(len+inplen+2);
					char* str = (char*)malloc(sz);
					snprintf(str, sz, "%s/%s", input, thisfile->d_name);
					char* name = getname(thisfile->d_name, len);
					addfile(filelist, str, name);
				}
			}
		}
		
		closedir(dir);

		if(filelist->filecount <= 0) {
			fprintf(stderr, "Directory doesn't have any .vm file\n");
			exit(1);
		}

		filelist->output = getoutname(input, inplen, isitdir);
	}
	else {
		int extind = inplen - 3;
		if(isdotvm(input, extind)){
			char* name = getname(input, inplen);
			char* f = heapstr(input, inplen);
			addfile(filelist, f, name);
		}
		else {
			fprintf(stderr, "Input file must be named like 'Xxx.vm'\n");
			exit(1);
		}
		
		filelist->output = getoutname(input, inplen, isitdir);
	}
	
	return filelist;
}

void freetranslationlist(TRANSLATIONLIST* ls) {
	for(int i = 0; i < ls->filecount; i++) {
		free(ls->files[i]);
		free(ls->fnames[i]);
	}
	free(ls->files);
	free(ls->fnames);
	free(ls->output);
	free(ls);
}

int main(int argc, char* argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Usage: %s {file}\n", argv[0]);
		return 1;
	}

	TRANSLATIONLIST* ls = getfiles(argv[1]);
	FILE* output = fopen(ls->output, "w");

	for(int i = 0; i < BOOTSTRAPN; i++) {
		fprintf(output, "%s\n", bootstrapcode[i]);
	}

	for(int i = 0; i < ls->filecount; i++) {
		// file name validating
		char* fname = ls->fnames[i];
		
		FILE* input = fopen(ls->files[i], "r");

		if(input == NULL) {
			fprintf(stderr, "%s\n", strerror(errno));
			return errno;
		}

		// parsing
		PARSER* p = mkparser(input);
		parse(p);

		// translating
		TRANSLATOR* t = mktranslator(p->lns, fname);
		translate(t);
		freeparser(p);
		
		printlns(t->output, output);

		// freeing rest
		freetranslator(t);
	}

	freetranslationlist(ls);
	fclose(output);
	return 0;
}
