CC=gcc
LD=gcc

HDIR=../../include

CFLAGS = -O0 -Wall -I$(HDIR)
V1190LIB = v792.o
LDFLAGS = -lvme $(V1190LIB)


TARGET = v792_test  \
         v792_reset \
	 v792_dump_data \
	 v792_thr \

all: v792.o $(TARGET)

clean:
	rm -f $(TARGET) *.o
