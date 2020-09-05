HOME =/home/quser
CC   = gcc
FC   = g77
CFLAG = -O0 -Wall 
LIB=-L${HOME}/udaq/lib
INC=-I${HOME}/udaq/include
LDFLAGS = -lvme
TARGET = librpv.o liblrm.o libcam.o libc256.o libvts801.o
.c.o:
	$(CC) $(CFLAG) -c $< $(INC) $(LIB)
lib: $(TARGET)
	rm -f libvmeio.a
	ar rcv libvmeio.a $(TARGET)
	mv libvmeio.a $(HOME)/udaq/lib
clean:
	rm -f *~ core *.o
