CC = gcc
CFLAGS = -O0 -Wall -I../../include
LDFLAGS = -lvme
TARGET=v830

$(TARGET):

scaler:scaler.c scaler.h
	$(LINK.c) -o v830 scaler.c

clean:
	rm -f *~ ${TARGET}
