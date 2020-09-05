CC=gcc
LD=gcc

HDIR=../../include
LIBDIR=../../lib

CFLAGS = -O0 -Wall -I$(HDIR)
MyRIADLIB = myriad.o
LDFLAGS = -lvme


TARGET = myriad_status \
	myriad_control

all: $(TARGET)

$(TARGET): $(MyRIADLIB)

myriad.o: myriad.c $(HDIR)/myriad.h
	$(COMPILE.c) -o myriad.o myriad.c
	cp myriad.o $(LIBDIR)

clean:
	rm -f $(TARGET) *.o
