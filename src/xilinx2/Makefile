# Makefile for Xilinx

CC=gcc
LD=gcc

CAMLIB = ../../lib/camlib2.o
BINDIR = ../../bin

CFLAGS = -O0 -Wall -I../../include
LDFLAGS = -lvme $(CAMLIB)

TARGET = Xilinxload 

all: $(TARGET)
	cp -a $(TARGET) $(BINDIR)

Xilinxload:  Xilinxload.o $(CAMLIB)
	$(LINK.c) -o $@ Xilinxload.o

clean:
	rm -f $(TARGET) *~ *.o

