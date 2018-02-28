# Makefile for the project

COMPILER_TYPE= gnu

CC= gcc

PROG= robinix

#Flag added to allow for using of asprintf without compilation warnings
CPPFLAGS += -D_POSIX_SOURCE

#Old manual adding option (this way is automatic and thus, better)
#SRCS= lab5.c test5.c


#Adds all *.c files for compiling by using the ls command
SRCS!= ls *.c

#For adding more files with a different extension manually
#Commented out for the time being because it is for now unnecessary
#SRCS+= <other files to add manually>
SRCS+= rtc_assembly.S
SRCS+= keyboard_assembly.S
SRCS+= mouse_assembly.S

CFLAGS= -Wall

DPADD+= ${LIBDRIVER} ${LIBSYS}
LDADD+= -llm -ldriver -lsys

LDFLAGS+= -L .

MAN=

.include <bsd.prog.mk>
.include <bsd.gcc.mk>
