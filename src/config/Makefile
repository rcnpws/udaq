CC=gcc
LD=gcc

HDIR=../../include
LIBDIR=../../lib

CFLAGS = -O0 -Wall -I$(HDIR) -lm -DLinux
LDFLAGS = -lm


TARGET = config.o \
	config_test

all: $(TARGET)

config.o: config.c
	$(COMPILE.c) $< -o $@
	cp -a config.o $(LIBDIR)

config_test: config_test.c config.o
	$(LINK.c) $< config.o -o $@

clean:
	rm -f $(TARGET) *.o
