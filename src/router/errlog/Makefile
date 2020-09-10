# Make file for Error Log
# Copyright (C) 1995  A. Tamii
# Author:   A. Tamii
#	Facility: Department of Physics, Kyoto University 
#	          & Research Center for Nuclear Physics
# Created:  28-JUL-1995 by A. Tamii

#.SILENT:

#******************
#* Compiler etc.  *
#******************
# general
LDFLAGS =
CFLAGS  = -g
#CC = cc
#LD = cc
CC = gcc
LD = gcc
AR = ar

#*************************
#* Directories           *
#*************************
SDIR = src
HDIR = header
ODIR = obj
EDIR = bin
LDIR = lib
GHDIR = ../header
GLDIR = ../lib
GEDIR = ../bin

#*************************
#* Default Rules         *
#*************************
# $(ODIR)/%.o : $(SDIR)/%.c
# 	$(COMPILE.c) $< -o $@
# 	$(LINT) $(CFLAGS) $<

#*************************
#* Command Line Switches *
#*************************
#UNAME   = `uname`
LFLAGS  = -L.
ARFLAGS = rc
#CFLAGS  = -I$(HDIR) -I$(GHDIR) -D`uname -s` -D`uname -m`
CFLAGS  = -I$(HDIR) -I$(GHDIR) -D`uname -s`

#*************************
#* Target Dependencies   *
#*************************
target: $(EDIR)/errlog $(ODIR)/errloglib.o $(GHDIR)/errlog.h

all: clean target

$(ODIR)/errloglib.o: $(SDIR)/errloglib.c
	$(CC) -c $(CFLAGS) $? -o $@
	cp $(ODIR)/errloglib.o $(GLDIR)

$(ODIR)/errlog.o: $(SDIR)/errlog.c
	$(CC) -c $(CFLAGS) $? -o $@

$(EDIR)/errlog: $(ODIR)/errlog.o
	$(LD) $? -o $@
	cp $@ $(GEDIR)/errlog

$(GHDIR)/errlog.h: $(HDIR)/errlog.h
	cp $? $@

clean:
	rm -f core $(EDIR)/errlog $(ODIR)/*.o
