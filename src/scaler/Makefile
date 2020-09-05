CC = gcc
CFLAGS = -O0 -Wall -I../../include
LDFLAGS = -lvme
TARGET=scaler

$(TARGET):

scaler:scaler.c scaler.h
	$(LINK.c) -o scaler scaler.c

clean:
	rm -f *~ ${TARGET}
