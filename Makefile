FILES = parser.c main.c translator.c util.c
INCLUDES = -I.
CFLAGS = -std=c99
OUTFILE = vmtranslator

main: ${FILES}
	${CC} ${CFLAGS} ${INCLUDES} -o ${OUTFILE} ${FILES}
