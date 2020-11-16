CC=gcc
CFLAGS=-Wall -pedantic -std=c99 -g3

Comments: comments.o
	${CC} ${CCFLAGS} -o $@ $^ -lm

comments.o: comments.c comments.h

CLI: cli.o
	${CC} ${CCFLAGS} -o $@ $^ -lm

cli.o: cli.c