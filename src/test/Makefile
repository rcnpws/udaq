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


LINK=   -lvme


# For blp collect
#SRC_COL=grcollect.c libc256.c grlib.c errlog.c ring_buf.c collect_tcp.c collect_data.c
#SRC_COL=collect_data.c librpv.c
SRC_COL=collect_data.c 
OBJ_COL=${SRC_COL:.c=.o}


# Default rule
.c.o:
	${CC} ${CFLAGS} ${INC} ${LIB}  -c $<
test:${OBJ_COL}
	${CC} ${CFLAGS} -o rpv_test ${OBJ_COL} ${LIB} ${INC} ${LINK} 
clean:
	rm -f *~ core *.o

