# Makefile for nfa

CC=gcc
LD=gcc

CAMLIB = ../../lib/camlib.o
BINDIR = ../../bin

CFLAGS = -O0 -Wall -I../../include
LDFLAGS = -lvme $(CAMLIB)

TARGET = nfa

all: $(TARGET)
	cp -a $(TARGET) $(BINDIR)

nfa: nfa.o  $(CAMLIB)
	$(LINK.c) -o $@ nfa.o

clean:
	rm -f $(TARGET) *~ *.o

