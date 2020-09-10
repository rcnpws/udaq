CC   = cc
FC   = xlf
CFLAGS = 
INC = -I${HOME}/udaq/include
LINK= 
SRC = tcp_connect.c wrapsock.c wraplib.c writen.c error.c \
      readn.c wrapunix.c tcp_listen.c wrapipc.c
OBJ = ${SRC:.c=.o}
.c.o:
	$(CC) $(CFLAGS) -c $< ${INC}
lib:${OBJ}
	\rm -f libtcp.a
	ar rcv libtcp.a ${OBJ}
#	cp libtcp.a ${HOME}/udaq/lib/
clean:
	\rm -f *~ core *.o

