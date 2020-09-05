# Makefile for MLU

CC=gcc
LD=gcc

CAMLIB = ../../lib/camlib.o
BINDIR = ../../bin

CFLAGS = -O0 -Wall -I../../include
LDFLAGS = -lvme $(CAMLIB)

TARGET = mluDump mluLoad2372_xuv mluLoad2372_uv
 
all: $(TARGET)
	cp -a $(TARGET) $(BINDIR)

mluDump:  mluDump.o $(CAMLIB)
	$(LINK.c) -o $@ mluDump.o

mluLoad2372_xuv:  mluLoad2372_xuv.o $(CAMLIB)
	$(LINK.c) -o $@ mluLoad2372_xuv.o

mluLoad2372_uv:  mluLoad2372_uv.o $(CAMLIB)
	$(LINK.c) -o $@ mluLoad2372_uv.o

clean:
	rm -f $(TARGET) *~ *.o

