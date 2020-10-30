FILES = parser.c main.c
INCLUDES = -I.
CFLAGS = -std=c99
OUTFILE = vmtranslator

main: ${FILES}
	${CC} ${CFLAGS} ${INCLUDES} -o ${OUTFILE} ${FILES}
