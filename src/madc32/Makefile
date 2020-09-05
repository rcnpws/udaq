CC=gcc
LD=gcc

HDIR=../../include
LIBDIR=../../lib

CFLAGS = -O0 -Wall -I$(HDIR)
MADC32LIB = madc32.o
LDFLAGS = -lvme -lm


TARGET = madc32_status \
	madc32_dump_data \
	madc32_threshold \
	madc32_init \

all: madc32.o $(TARGET)
	cp -f $(TARGET) ${HOME}/udaq/bin

$(TARGET): $(MADC32LIB)

madc32.o: madc32.c $(HDIR)/madc32.h
	$(COMPILE.c) -o madc32.o madc32.c
	cp madc32.o $(LIBDIR)

clean:
	rm -f $(TARGET) *.o
