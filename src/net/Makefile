HOME =/home/quser
CC   = gcc
FC   = g77
#CFLAGS = -O -xarch=v9
CFLAGS = -O0 -Wall
INC = -I${HOME}/udaq/include
#LINK= -lsocket -lnsl
SRC = tcp_connect.c wrapsock.c wraplib.c writen.c error.c \
      readn.c wrapunix.c tcp_listen.c wrapipc.c
OBJ = ${SRC:.c=.o}
.c.o:
	$(CC) $(CFLAGS) -c $< ${INC}
lib:${OBJ}
	rm -f libtcp.a
	ar rcv libtcp.a ${OBJ}
	mv libtcp.a ${HOME}/udaq/lib/
clean:
	rm -f *~ core *.o

