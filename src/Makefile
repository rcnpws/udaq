# Makefile

SUBDIRS = camlib nfa init3377 xilinx pcos mlu k2917 rpv130 
BIRDIR  = ../bin
HDIR    = ../include
LIBDIR  = ../lib

MAKE = make

target:
	list='$(SUBDIRS)'; for subdir in $$list; do \
		$(MAKE) -C $$subdir;\
	done

clean:
	list='$(SUBDIRS)'; for subdir in $$list; do \
		$(MAKE) clean -C $$subdir;\
	done
