CC = gcc
CFLAGS = -O0 -Wall -I../../include
LDFLAGS = -lvme
TARGET=vme_reset

$(TARGET):

vme_reset:vme_reset.c
	$(LINK.c) -o vme_reset vme_reset.c

clean:
	rm -f *~ ${TARGET}
