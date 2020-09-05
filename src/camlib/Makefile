CC=gcc
LD=gcc

HDIR=../../include

CFLAGS = -O0 -Wall -I$(HDIR)
LDFLAGS = 

TARGET = camlib.o

all: $(TARGET)

camlib.o: camlib.c $(HDIR)/camlib.h
	$(COMPILE.c) -o camlib.o camlib.c 
	cp -a camlib.o ../../lib
clean:
	rm -f $(TARGET) *~ *.o

