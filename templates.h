#ifndef templates
#define templates

#define TPUSHN 11
char* tpush[TPUSHN] = {
	"",
	"",
	"",
	"",
	"A=D+A",
	"D=M",
	"@SP",
	"A=M",
	"M=D",
	"@SP",
	"M=M+1"
};

#define TPUSHCONSN 8
char* tpushcons[TPUSHCONSN] = {
	"",
	"",
	"D=A",
	"@SP",
	"A=M",
	"M=D",
	"@SP",
	"M=M+1"
};

#define TPUSHSTATN 8
char* tpushstat[TPUSHSTATN] = {
	"",
	"",
	"D=M",
	"@SP",
	"A=M",
	"M=D",
	"@SP",
	"M=M+1"
};

#define TPUSHTEMP 8
char* tpushtemp[TPUSHTEMP] = {
	"",
	"",
	"D=M",
	"@SP",
	"A=M",
	"M=D",
	"@SP",
	"M=M+1"
};

#define TPOPN 14
char* tpop[TPOPN] = {
	"",
	"",
	"",
	"",
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

#define TPOPSTATN 8
char* tpopstat[TPOPSTATN] = {
	"",
	"@SP",
	"D=M",
	"M=M-1",
	"A=D",
	"D=M",
	"",
	""
};

#define TPOPTEMPN 7
char* tpoptemp[TPOPTEMPN] = {
	"",
	"@SP",
	"M=M-1",
	"A=M",
	"D=M",
	"",
	""
};

#endif
