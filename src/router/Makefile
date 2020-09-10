# Make file for Router
# Copyright (C) 1995  A. Tamii
# Author:   A. Tamii
#	Facility: Department of Physics, Kyoto University 
#	          & Research Center for Nuclear Physics
# Created:  24-JAN-1995 by A. Tamii (for Solaris 2.3)

#.SILENT:

#******************
#* Compiler etc.  *
#******************
# general
LDFLAGS =
CFLAGS  = -g
CC = cc
LD = cc
#CC = gcc
#LD = gcc
AR = ar

#*************************
#* Directories           *
#*************************
SDIR = src
HDIR = header
ODIR = obj
EDIR = bin
LDIR = lib
GHDIR = errlog/header
GLDIR = lib
GEDIR = ../../bin

#*************************
#* Default Rules         *
#*************************
# $(ODIR)/%.o : $(SDIR)/%.c
# 	$(COMPILE.c) $< -o $@
# 	$(LINT) $(CFLAGS) $<

# $(GHDIR)/%.h : $(HDIR)/%.h
# 	cp $< $@
# 

#*************************
#* Command Line Switches *
#*************************
LFLAGS  = -L.
ARFLAGS = rc
#CFLAGS  = -I$(HDIR) -I$(GHDIR) -I$(RFM)/header -D`uname -s` -D`uname -m`
CFLAGS  = -I$(HDIR) -I$(GHDIR) -I$(RFM)/header -D`uname -s`

#*************************
#* Target Dependencies   *
#*************************
MODULES =  $(ODIR)/router.o $(ODIR)/listen.o

GLHDR = $(GHDIR)/router.h $(GHDIR)/router_priv.h

router: $(EDIR)/router $(GLHDR)

all: clean router

$(ODIR)/router.o: $(SDIR)/router.c
	$(CC) -c $(CFLAGS) $? -o $@

$(ODIR)/listen.o: $(SDIR)/listen.c
	$(CC) -c $(CFLAGS) $? -o $@

$(ODIR)/router_lib.o: $(SDIR)/router_lib.c
	$(CC) -c $(CFLAGS) $? -o $@
	cp $@ $(GLDIR)/router_lib.o

$(GHDIR)/router.h: $(HDIR)/router.h
	cp $? $@

$(GHDIR)/router_priv.h: $(HDIR)/router_priv.h
	cp $? $@

$(EDIR)/router: $(MODULES) $(ODIR)/router_lib.o $(GLDIR)/errloglib.o
	$(LD) $(MODULES) $(ODIR)/router_lib.o $(GLDIR)/errloglib.o -o $@
	cp $@ $(GEDIR)/router

clean:
	rm -f core $(EDIR)/router $(ODIR)/*.o

