#ifndef templates
#define templates

#define TPUSHN 10
char* tpush[TPUSHN] = {
	"",
	"",
	"",
	"",
	"A=D+A",
	"D=M",
	"@SP",
	"M=M+1",
	"A=M-1",
	"M=D"
};

#define TPUSHCONSN 7
char* tpushcons[TPUSHCONSN] = {
	"",
	"",
	"D=A",
	"@SP",
	"M=M+1",
	"A=M-1",
	"M=D"
};

#define TPUSHSTATN 7
char* tpushstat[TPUSHSTATN] = {
	"",
	"",
	"D=M",
	"@SP",
	"M=M+1",
	"A=M-1",
	"M=D"
};

#define TPUSHTEMPN TPUSHSTATN
char** tpushtemp = tpushstat;

#define TPUSHPOINTERN TPUSHSTATN
char** tpushpointer = tpushstat;

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

#define TPOPSTATN 7
char* tpopstat[TPOPSTATN] = {
	"",
	"@SP",
	"M=M-1",
	"A=M+1",
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

#define TPOPPOINTERN 7
char* tpoppointer[TPOPPOINTERN] = {
	"",
	"@SP",
	"M=M-1",
	"A=M",
	"D=M",
	"",
	"M=D"
};

#endif
