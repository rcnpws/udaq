CC=gcc
LD=gcc

HDIR=../../include
LIBDIR=../../lib

CFLAGS = -O0 -Wall -I$(HDIR)
V1190LIB = v1190.o
LDFLAGS = -lvme


TARGET = v1190_trig_match_mode \
	v1190_cont_storage_mode \
	v1190_status \
	v1190_dump \
	v1190_software_trigger \
	v1190_en_head_trailer \
	v1190_dis_head_trailer \
	v1190_read_head_trailer \
	v1190_reset \
	v1190_init \
	v1190_mcst_cont \
	v1190_check_data \
	v1190_window \
	v1190_read \
	v1190_hit \
	vme_sysreset \


all: v1190.o $(TARGET)
	cp -f $(TARGET) ${HOME}/udaq/bin

$(TARGET): $(V1190LIB)

v1190.o: v1190.c $(HDIR)/v1190.h
	$(COMPILE.c) -o v1190.o v1190.c
	cp v1190.o $(LIBDIR)

v1190_hit: v1190_hit.c
	$(LINK.c) $(V1190LIB) -lm v1190_hit.c -o v1190_hit

v1190_dump: v1190_dump.c v1190_dump_data.o
	$(LINK.c) $(V1190LIB) v1190_dump_data.o v1190_dump.c -o v1190_dump

clean:
	rm -f $(TARGET) *.o
