CC=gcc
LD=gcc

HDIR=../../include
LIBDIR=../../lib

CFLAGS = -O0 -Wall -I$(HDIR)
LIB = $(LIBDIR)/v1190.o $(LIBDIR)/madc32.o $(LIBDIR)/v977.o $(LIBDIR)/v830.o $(LIBDIR)/config.o
LDFLAGS = -lvme


TARGET = vmedaq tcp_send vmedaq_new

all: $(TARGET)
	cp -f $(TARGET) ${HOME}/udaq/bin

$(TARGET): $(LIB)

tcp_send: tcp_send.c
	$(LINK.c) $(LIB) -lm tcp_send.c -o tcp_send

vmedaq: vmedaq.c
	$(LINK.c) $(LIB) -lm vmedaq.c -o vmedaq

vmedaq_new: vmedaq_new.c
	$(LINK.c) $(LIB) -lm vmedaq_new.c -o vmedaq_new

clean:
	rm -f $(TARGET) *.o
