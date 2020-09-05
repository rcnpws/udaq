#
# Makefile for FORCE54T(Solaris8 64bit)
# GR collector
#
#      2003/08/18   M.Uchida
#
#
HOME = /home/quser
CC   = gcc
FC   = c77
#CFLAGS = -O2 -Wall
CFLAGS = -O0 -Wall

LIB=-L${HOME}/udaq/lib 
INC=-I${HOME}/udaq/include

LINK=  -lpthread -lrt -lvmeio -lvme
NLINK = -ltcp -lnsl  

# block size for data transfer
#BLK_BUF_SIZE=0x2000
BLK_BUF_SIZE=0x2000

# For blp collect
#SRC_COL=grcollect.c libc256.c grlib.c errlog.c ring_buf.c collect_tcp.c collect_data.c
SRC_COL=grcollect.c grlib.c errlog.c ring_buf.c collect_tcp.c collect_data.c
OBJ_COL=${SRC_COL:.c=.o}

# Default rule
.c.o:
	${CC} ${CFLAGS} ${INC} ${LIB} -DBLK_BUF_SIZE=$(BLK_BUF_SIZE) -c $<
grcollect:${OBJ_COL}
	${CC} ${CFLAGS} -o ${HOME}/udaq/bin/$@ ${OBJ_COL} -DBLK_BUF_SIZE=$(BLK_BUF_SIZE) ${LIB} ${INC} ${LINK} ${NLINK} 
grdump_local:${OBJ_DMPL}
	${CC} ${CFLAGS} -o $@ ${OBJ_DMPL} ${LIB} ${INC} ${LINK} ${NLINK} 
install:
	cp grcollect    ../../bin/
clean:
	rm -f *~ core *.o

