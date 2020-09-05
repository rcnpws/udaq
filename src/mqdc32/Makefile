CC=gcc
LD=gcc

HDIR=../../include
LIBDIR=../../lib

CFLAGS = -O0 -Wall -I$(HDIR)
MQDC32LIB = mqdc32.o
LDFLAGS = -lvme -lm


TARGET = mqdc32_status \
	mqdc32_dump_data \
	mqdc32_threshold \
	mqdc32_init \

all: mqdc32.o $(TARGET)
	cp -f $(TARGET) ${HOME}/udaq/bin

$(TARGET): $(MQDC32LIB)

mqdc32.o: mqdc32.c $(HDIR)/mqdc32.h
	$(COMPILE.c) -o mqdc32.o mqdc32.c
	cp mqdc32.o $(LIBDIR)

clean:
	rm -f $(TARGET) *.o
