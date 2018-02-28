# Makefile for lab4

COMPILER_TYPE= gnu

CC= gcc

PROG= lab5
#Old manual adding option (this way is automatic and thus, better)
#SRCS= lab5.c test5.c


#Adds all *.c files for compiling by using the ls command
SRCS!= ls *.c

#For adding more files with a different extension manually
#Commented out for the time being because it is for now unnecessary
#SRCS+= <other files to add manually>

CFLAGS= -Wall

DPADD+= ${LIBDRIVER} ${LIBSYS}
LDADD+= -llm -ldriver -lsys

LDFLAGS+= -L .

MAN=

.include <bsd.prog.mk>
.include <bsd.gcc.mk>
