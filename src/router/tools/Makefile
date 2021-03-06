# Make file for Router Tools
# Copyright (C) 1995  A. Tamii
# Author:   A. Tamii
#	Facility: Department of Physics, Kyoto University 
#	          & Research Center for Nuclear Physics
# Created:  24-JAN-1995 by A. Tamii (for Solaris 2.3)
# Version 2.0  19-JAN-1997 by A. Tamii

#.SILENT:

#******************
#* Compiler etc.  *
#******************
# general
#CC = cc
#LD = cc
CC = gcc
LD = gcc
AR = ar

#*************************
#* Directories           *
#*************************
SDIR = .
HDIR = ../header
ODIR = .
EDIR = .
LDIR = ../lib
GHDIR = ../header
GLDIR = ../lib
GEDIR = ../../../bin

#*************************
#* Command Line Switches *
#*************************
#UNAME   = `uname -s`
LDFLAGS = -L. -L$(GLDIR) $(GLDIR)/router_lib.o
ARFLAGS = rc
#CFLAGS  = -I$(HDIR) -I$(GHDIR) -D`uname -s` -D`uname -m` -g
CFLAGS  = -I$(HDIR) -I$(GHDIR) -D`uname -s` -g

#--- Solaris ---
#TCPLIBS = -lsocket -lnsl  # Solaris 2.x
#RL = 

#--- Degital UNIX ---
TCPLIBS =                # BSD
RL = ranlib $@

#*************************
#* Target Dependencies   *
#*************************
#TARGET =  router_send router_dump \
#	router_save router_scaler router_daqinfo\
#	router_write router_tcp tcp_router daqinfo\
#          tcp_cat tcp_cat_v1 tcp_save tcp_dump tcp_send \
#          router_fread router_message router_check tcp_read
TARGET =  router_send router_dump \
	router_save router_scaler \
	router_write router_tcp tcp_router \
          tcp_cat tcp_cat_v1 tcp_save tcp_dump tcp_send \
          router_fread router_message tcp_read
#         router_cat

tools: $(TARGET)
	cp $? $(GEDIR)

all: clean tools

router_tcp: router_tcp.c $(GLDIR)/errloglib.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(TCPLIBS) router_tcp.c $(GLDIR)/errloglib.o -o $@

tcp_dump: tcp_dump.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(TCPLIBS) tcp_dump.c -o $@

tcp_cat: tcp_cat.c tcplib.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(TCPLIBS) tcp_cat.c -o $@

tcp_cat_v1: tcp_cat_v1.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(TCPLIBS) tcp_cat_v1.c -o $@

tcp_save: tcp_save.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(TCPLIBS) tcp_save.c -o $@

tcp_send: tcp_send.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(TCPLIBS) tcp_send.c -o $@

tcp_router: tcp_router.c $(GLDIR)/errloglib.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(TCPLIBS) tcp_router.c $(GLDIR)/errloglib.o -o $@

check_format_lib.o: check_format_lib.c
	$(CC) -c $(CFLAGS) -I$(GHDIR) check_format_lib.c -o $@

check_adc.o: check_adc.c
	$(CC) -c $(CFLAGS) -I$(GHDIR) check_adc.c -o $@

check_format: check_format.c check_format_lib.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(GHDIR) check_format.c check_format_lib.o -o $@

router_check: router_check.c check_format_lib.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(GHDIR) router_check.c \
		check_format_lib.o -o $@

router_check_format: router_check_format.c check_format_lib.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(GHDIR) router_check_format.c \
		check_format_lib.o -o $@

router_lr3377: router_lr3377.c $(GLDIR)/libmod.a
	$(CC) $(CFLAGS) $(LDFLAGS)  -I$(GHDIR) router_lr3377.c -lmod -o $@

router_lr3377_hit: router_lr3377_hit.c $(GLDIR)/libmod.a
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(GHDIR) router_lr3377_hit.c -lmod -o $@

router_check_adc: router_check_format.c check_adc.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(GHDIR) router_check_format.c check_adc.o -o $@

libdaqtcp.a: tcplib.o
	ar -r $@ $?
	$(RL)
	cp $@ $(GLDIR)

tcplib.o: tcplib.c
	$(CC) -I../header $(CFLGAS) -c tcplib.c -o $@
	cp $@ $(GLDIR)

tcp_read: tcp_read.c libdaqtcp.a
	$(CC) $(CFLAGS) $(LDFLAGS)  $(TCPLIBS) tcp_read.c -ldaqtcp -o $@

assign_j11.o: assign_j11.c
	$(CC) -c $(CFLAGS) -I$(GHDIR) assign_j11.c -o $@

router_dump: router_dump.c $(GLDIR)/errloglib.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(GHDIR) $(TCPLIBS) router_dump.c \
		$(GLDIR)/errloglib.o -o $@

router_save: router_save.c assign_j11.o $(GLDIR)/errloglib.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(GHDIR) $(TCPLIBS) router_save.c assign_j11.o \
		$(GLDIR)/errloglib.o -o $@

router_fread: router_fread.c assign_j11.o $(GLDIR)/errloglib.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(GHDIR) $(TCPLIBS) router_fread.c \
    assign_j11.o $(GLDIR)/errloglib.o -o $@

router_scaler: router_scaler.c assign_j11.o $(GLDIR)/errloglib.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(GHDIR) $(TCPLIBS) router_scaler.c \
		assign_j11.o $(GLDIR)/errloglib.o -o $@

router_daqinfo: router_daqinfo.c assign_j11.o $(GLDIR)/errloglib.o
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(GHDIR) $(TCPLIBS) router_daqinfo.c \
		assign_j11.o $(GLDIR)/errloglib.o -o $@

daqinfo: daqinfo.c 
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(GHDIR) daqinfo.c -o $@

rfm_router: rfm_router.c $(GLDIR)/errloglib.o $(GLDIR)/rfm_lib.o
	$(CC) $(CFLAGS) $(LDFLAGS) rfm_router.c $(GLDIR)/errloglib.o $(GLDIR)/rfm_lib.o -o $@

clean:
	rm -f core *.o *.a $(TARGET)
