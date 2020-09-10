#
# Makefile for AIX4.3(xlc,xlf)
#
#    2002/02/08   M.Uchida
#
#    2020/09/08   A. Tamii, Modified for vmepla
CC=cc
FC=xlf
CFLAGS = 

# DAQ library and header
#LIB=-L${HOME}/udaq/lib 
#INC=-I${HOME}/udaq/include 
LIB=-L./tcplib/
INC=-I.
NLINK = -ltcp -lpthread

# For blp save
SRC_CTRL=grctrl.c 
OBJ_CTRL=${SRC_CTRL:.c=.o}

all:grctrl
.c.o:
	${CC} ${CFLAGS} -c $< ${LIB} ${INC}
.f.o:
	${FC} -g -O3 -c $<
grctrl: ${OBJ_CTRL}
	${CC} ${CFLAG} -o $@  ${OBJ_CTRL}  ${LIB} ${INC} ${NLINK}
install: grctrl
	cp -a grctrl    ../../bin
clean:
	rm -f *~ core *.o
