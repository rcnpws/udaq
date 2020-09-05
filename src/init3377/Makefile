# Makefile for init3377

CC=gcc
LD=gcc

CAMLIB = ../../lib/camlib.o
BINDIR = ../../bin

CFLAGS = -O0 -Wall -I../../include
LDFLAGS = -lvme $(CAMLIB)

TARGET = init3377_gr init3377_las init3377_las_uv init3377_gr_e247 init3377_las_e247 init3377_ssd_e404 init3377_las_e423

GR=gr.o
LAS=las.o
SSD=ssd.o
 
all: $(TARGET)
	cp -a $(TARGET) $(BINDIR)

init3377_gr:  init3377.o module_gr.o $(GR) $(CAMLIB)
	$(LINK.c) $(GR) -o $@ init3377.o module_gr.o

init3377_las: init3377.o module_las_uv.o $(LAS)  $(CAMLIB)
	$(LINK.c) $(LAS) -o $@ init3377.o module_las_uv.o

init3377_las_uv: init3377.o module_las_uv.o $(LAS)  $(CAMLIB)
	$(LINK.c) $(LAS) -o $@ init3377.o module_las_uv.o

init3377_las_xu: init3377.o module_las_xu.o $(LAS)  $(CAMLIB)
	$(LINK.c) $(LAS) -o $@ init3377.o module_las_xu.o

init3377_gr_e247: init3377.o module_gr_e247.o  $(GR) $(CAMLIB)
	$(LINK.c) $(GR) -o $@ init3377.o module_gr_e247.o

init3377_las_e247: init3377.o module_las_e247.o $(LAS) $(CAMLIB)
	$(LINK.c) $(LAS) -o $@ init3377.o module_las_e247.o

init3377_ssd_e404: init3377.o module_ssd_e404.o $(SSD) $(CAMLIB)
	$(LINK.c) $(SSD) -o $@ init3377.o module_ssd_e404.o

# init3377_las_e423: init3377_e423.o module_las_e423.o $(LAS) $(CAMLIB)
#	$(LINK.c) $(LAS) -o $@ init3377_e423.o module_las_e423.o

init3377_las_e423: init3377_e423.o module_las_uv.o $(LAS) $(CAMLIB)
	$(LINK.c) $(LAS) -o $@ init3377_e423.o module_las_uv.o

clean:
	rm -f $(TARGET) *~ *.o

