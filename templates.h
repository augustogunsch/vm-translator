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
	"M=M+1",
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
	"M=M+1",
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
	"M=M+1",
};

#define TPUSHTEMPN TPUSHSTATN
char** tpushtemp = tpushstat;

#define TPUSHPOINTERN TPUSHSTATN
char** tpushpointer = tpushstat;

#define TPOPN 13
char* tpop[TPOPN] = {
	"",
	"",
	"",
	"",
	"D=D+A",
	"@R13",
	"M=D",
	"@SP",
	"AM=M-1",
	"D=M",
	"@R13",
	"A=M",
	"M=D"
};

#define TPOPSTATN 6
char* tpopstat[TPOPSTATN] = {
	"",
	"@SP",
	"AM=M-1",
	"D=M",
	"",
	""
};

#define TPOPTEMPN TPOPSTATN
char** tpoptemp = tpopstat;

#define TPOPPOINTERN TPOPSTATN
char** tpoppointer = tpopstat;

#define TARITHN 6
char* tarith[TARITHN] = {
	"",
	"@SP",
	"AM=M-1",
	"D=M",
	"A=A-1",
	""
};

#define TNEGN 4
char* tneg[TNEGN] = {
	"",
	"@SP",
	"A=M-1",
	"M=-M",
};

#define TNOTN 4
char* tnot[TNOTN] = {
	"",
	"@SP",
	"A=M-1",
	"M=!M",
};

#define TCOMPN 13
char* tcomp[TCOMPN] = {
	"",
	"@SP",
	"AM=M-1",
	"D=M",
	"A=A-1",
	"D=D-M",
	"M=-1",
	"",
	"",
	"@SP",
	"A=M-1",
	"M=0",
	""
};

#define TLABELN 2
char* tlabel[TLABELN] = {
	"",
	""
};

#define TGOTON 3
char* tgoto[TGOTON] = {
	"",
	"",
	"0;JMP"
};

#define TIFGOTON 6
char* tifgoto[TIFGOTON] = {
	"",
	"@SP",
	"AM=M-1",
	"D=M",
	"",
	"D;JNE"
};

#define TCALLSTARTN 8
char* tcallstart[TCALLSTARTN] = {
	"",
	"",
	"D=A",
	"@SP",
	"A=M",
	"M=D",
	"@SP",
	"M=M+1",
};

#define TCALLPUSHN 7
char* tcallpush[TCALLPUSHN] = {
	"",
	"D=M",
	"@SP",
	"A=M",
	"M=D",
	"@SP",
	"M=M+1",
};

#define TCALLSETARGN 8
char* tcallsetarg[TCALLSETARGN] = {
	"@SP",
	"D=M",
	"@LCL",
	"M=D",
	"",
	"D=D-A",
	"@ARG",
	"M=D"
};

#define TCALLJMPN 3
char* tcalljmp[TCALLJMPN] = {
	"",
	"0;JMP",
	""
};
	
#define TFRAMEVARSN 4
char* tframevars[TFRAMEVARSN] = {
	"@LCL",
	"@ARG",
	"@THIS",
	"@THAT"
};

#define TFUNCTIONN 2
char* tfunction[TFUNCTIONN] = {
	"",
	""
};

#define TFUNCTIONPUSHN 5
char* tfunctionpush[TFUNCTIONPUSHN] = {
	"@SP",
	"A=M",
	"M=0",
	"@SP",
	"M=M+1"
};

#define TSTARTRETURNN 18
char* tstartreturn[TSTARTRETURNN] = {
	"",
	"@LCL",
	"D=M",
	"@5",
	"A=D-A",
	"D=M",
	"@R13",
	"M=D",
	"@SP",
	"A=M-1",
	"D=M",
	"@ARG",
	"A=M",
	"M=D",
	"@ARG",
	"D=M+1",
	"@SP",
	"M=D"
};

#define TRETPOPN 5
char* tretpop[TRETPOPN] = {
	"@LCL",
	"AM=M-1",
	"D=M",
	"",
	"M=D",
};

#define TENDRETURNN 3
char* tendreturn[TENDRETURNN] = {
	"@R13",
	"A=M",
	"0;JMP"
};

#endif
