HDIR=../../include
BINDIR=../../bin
LIBDIR=../../lib

CC = gcc
CFLAGS = -O0 -Wall -I$(HDIR)
LDFLAGS = -lvme
TARGETS =  v830 v830_read

all: $(TARGETS)

v830_read:v830_read.c $(HDIR)/v830.h $(LIBDIR)/v830.o
	$(LINK.c) -o v830_read v830_read.c $(LIBDIR)/v830.o
	cp v830_read $(BINDIR)

v830:v830.c $(HDIR)/v830.h
	$(COMPILE.c) -o v830.o v830.c
	cp v830.o $(LIBDIR)

clean:
	rm -f *~ ${TARGET}
