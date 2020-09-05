CC=gcc
LD=gcc

HDIR=../../include

CFLAGS = -O0 -Wall -I$(HDIR)
#CFLAGS = -O2 -Wall -I$(HDIR)
LDFLAGS = 

TARGET = camlib2.o

all: $(TARGET)

camlib2.o: camlib2.c $(HDIR)/camlib2.h
	$(COMPILE.c) -o camlib2.o camlib2.c 
	cp -a camlib2.o ../../lib
clean:
	rm -f $(TARGET) *~ *.o

