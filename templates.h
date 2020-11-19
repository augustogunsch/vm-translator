#ifndef templates
#define templates

typedef struct {
	char** items;
	int count;
} TEMPLATE;

char* tpushlns[] = {
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
TEMPLATE tpush = {
	.items = tpushlns,
	.count = sizeof(tpushlns) / sizeof(char*)
};

char* tpushconslns[] = {
	"",
	"",
	"D=A",
	"@SP",
	"A=M",
	"M=D",
	"@SP",
	"M=M+1",
};
TEMPLATE tpushcons = {
	.items = tpushconslns,
	.count = sizeof(tpushconslns) / sizeof(char*)
};

char* tpushstatlns[] = {
	"",
	"",
	"D=M",
	"@SP",
	"A=M",
	"M=D",
	"@SP",
	"M=M+1",
};
TEMPLATE tpushstat = {
	.items = tpushstatlns,
	.count = sizeof(tpushstatlns) / sizeof(char*)
};

TEMPLATE tpushtemp = {
	.items = tpushstatlns,
	.count = sizeof(tpushstatlns) / sizeof(char*)
};

TEMPLATE tpushpointer = {
	.items = tpushstatlns,
	.count = sizeof(tpushstatlns) / sizeof(char*)
};

char* tpoplns[] = {
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
TEMPLATE tpop = {
	.items = tpoplns,
	.count = sizeof(tpoplns) / sizeof(char*)
};

char* tpopstatlns[] = {
	"",
	"@SP",
	"AM=M-1",
	"D=M",
	"",
	""
};
TEMPLATE tpopstat = {
	.items = tpopstatlns,
	.count = sizeof(tpopstatlns) / sizeof(char*)
};

TEMPLATE tpoptemp = {
	.items = tpopstatlns,
	.count = sizeof(tpopstatlns) / sizeof(char*)
};

TEMPLATE tpoppointer = {
	.items = tpopstatlns,
	.count = sizeof(tpopstatlns) / sizeof(char*)
};

char* tarithlns[] = {
	"",
	"@SP",
	"AM=M-1",
	"D=M",
	"A=A-1",
	""
};
TEMPLATE tarith = {
	.items = tarithlns,
	.count = sizeof(tarithlns) / sizeof(char*)
};

char* tneglns[] = {
	"",
	"@SP",
	"A=M-1",
	"M=-M",
};
TEMPLATE tneg = {
	.items = tneglns,
	.count = sizeof(tneglns) / sizeof(char*)
};

char* tnotlns[] = {
	"",
	"@SP",
	"A=M-1",
	"M=!M",
};
TEMPLATE tnot = {
	.items = tnotlns,
	.count = sizeof(tnotlns) / sizeof(char*)
};

char* tcomplns[] = {
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
TEMPLATE tcomp = {
	.items = tcomplns,
	.count = sizeof(tcomplns) / sizeof(char*)
};

char* tlabellns[] = {
	"",
	""
};
TEMPLATE tlabel = {
	.items = tlabellns,
	.count = sizeof(tlabellns) / sizeof(char*)
};

char* tgotolns[] = {
	"",
	"",
	"0;JMP"
};
TEMPLATE tgoto = {
	.items = tgotolns,
	.count = sizeof(tgotolns) / sizeof(char*)
};

char* tifgotolns[] = {
	"",
	"@SP",
	"AM=M-1",
	"D=M",
	"",
	"D;JNE"
};
TEMPLATE tifgoto = {
	.items = tifgotolns,
	.count = sizeof(tifgotolns) / sizeof(char*)
};

char* tcallstartlns[] = {
	"",
	"",
	"D=A",
	"@SP",
	"A=M",
	"M=D",
	"@SP",
	"M=M+1",
};
TEMPLATE tcallstart = {
	.items = tcallstartlns,
	.count = sizeof(tcallstartlns) / sizeof(char*)
};

char* tcallpushlns[] = {
	"",
	"D=M",
	"@SP",
	"A=M",
	"M=D",
	"@SP",
	"M=M+1",
};
TEMPLATE tcallpush = {
	.items = tcallpushlns,
	.count = sizeof(tcallpushlns) / sizeof(char*)
};

char* tcallsetarglns[] = {
	"@SP",
	"D=M",
	"@LCL",
	"M=D",
	"",
	"D=D-A",
	"@ARG",
	"M=D"
};
TEMPLATE tcallsetarg = {
	.items = tcallsetarglns,
	.count = sizeof(tcallsetarglns) / sizeof(char*)
};

char* tcalljmplns[] = {
	"",
	"0;JMP",
	""
};
TEMPLATE tcalljmp = {
	.items = tcalljmplns,
	.count = sizeof(tcalljmplns) / sizeof(char*)
};
	
char* tframevarslns[] = {
	"@LCL",
	"@ARG",
	"@THIS",
	"@THAT"
};
TEMPLATE tframevars = {
	.items = tframevarslns,
	.count = sizeof(tframevarslns) / sizeof(char*)
};

char* tfunctionlns[] = {
	"",
	""
};
TEMPLATE tfunction = {
	.items = tfunctionlns,
	.count = sizeof(tfunctionlns) / sizeof(char*)
};

char* tfunctionpushlns[] = {
	"@SP",
	"A=M",
	"M=0",
	"@SP",
	"M=M+1"
};
TEMPLATE tfunctionpush = {
	.items = tfunctionpushlns,
	.count = sizeof(tfunctionpushlns) / sizeof(char*)
};

char* tstartreturnlns[] = {
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
TEMPLATE tstartreturn = {
	.items = tstartreturnlns,
	.count = sizeof(tstartreturnlns) / sizeof(char*)
};

char* tretpoplns[] = {
	"@LCL",
	"AM=M-1",
	"D=M",
	"",
	"M=D",
};
TEMPLATE tretpop = {
	.items = tretpoplns,
	.count = sizeof(tretpoplns) / sizeof(char*)
};

char* tendreturnlns[] = {
	"@R13",
	"A=M",
	"0;JMP"
};
TEMPLATE tendreturn = {
	.items = tendreturnlns,
	.count = sizeof(tendreturnlns) / sizeof(char*)
};

#endif
