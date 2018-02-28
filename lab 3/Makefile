# Makefile for lab3

COMPILER_TYPE= gnu

CC= gcc

PROG= lab3
#Old manual adding option (this way is automatic and thus, better)
#SRCS= lab3.c test3.c keyboard.c


#Adds all *.c files for compiling by using the ls command
SRCS!= ls *.c

#For adding more files with a different extension manually
#Commented out for the time being because it is for now unnecessary
#SRCS+= <other files to add manually>
SRCS+= keyboard_assembly.S

#Flag added because of sys_inb counting (suggested in class notes)
CPPFLAGS += -D LAB3

CFLAGS= -Wall

DPADD+= ${LIBDRIVER} ${LIBSYS}
LDADD+= -ldriver -lsys

LDFLAGS+= -L .

MAN=

.include <bsd.prog.mk>
.include <bsd.gcc.mk>
