# Makefile for PCOS

CC=gcc
LD=gcc

CAMLIB = ../../lib/camlib.o
BINDIR = ../../bin

CFLAGS = -O0 -Wall -I../../include
LDFLAGS = -lvme $(CAMLIB)

TARGET = pcos_setaddress pcos_delay pcos_threshold
 
all: $(TARGET)
	cp -a $(TARGET) $(BINDIR)

pcos_setaddress:  pcos_setaddress.o $(CAMLIB)
	$(LINK.c) -o $@ pcos_setaddress.o

pcos_delay:  pcos_delay.o $(CAMLIB)
	$(LINK.c) -o $@ pcos_delay.o

pcos_threshold:  pcos_threshold.o $(CAMLIB)
	$(LINK.c) -o $@ pcos_threshold.o

clean:
	rm -f $(TARGET) *~ *.o

