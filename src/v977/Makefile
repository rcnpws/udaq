CC=gcc
LD=gcc

HDIR=../../include
BINDIR=../../bin
LIBDIR=../../lib

CFLAGS = -O0 -Wall -I$(HDIR)
V1190LIB = v977.o
LDFLAGS = -lvme -lm $(V1190LIB)


TARGET = v977_test  \
	 v977_pulser \
	 v977_ope \

all: v977.o $(TARGET)

v977.o:v977.c $(HDIR)/v977.h
	$(COMPILE.c) -o v977.o v977.c
	cp v977.o $(LIBDIR)

clean:
	rm -f $(TARGET) *.o
