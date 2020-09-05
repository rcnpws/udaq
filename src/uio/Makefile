CC = gcc
CFLAGS = -O0 -Wall -I../../include
LDFLAGS = -lvme
TARGET=uio

$(TARGET):

uio:uio.c uio.h
	$(LINK.c) -o uio uio.c

clean:
	rm -f *~ ${TARGET}
