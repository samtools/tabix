PROG=		bgzip tabix

all: $(PROG)


# Adjust $(HTSDIR) to point to your top-level htslib directory
HTSDIR = ../htslib
include $(HTSDIR)/htslib.mk
HTSLIB = $(HTSDIR)/libhts.a

CC=			gcc
CFLAGS=		-g -Wall -O2 -fPIC #-m64 #-arch ppc
DFLAGS=		-D_FILE_OFFSET_BITS=64 -D_USE_KNETFILE -DBGZF_CACHE
OBJS=		tabix.o bgzip.o
INCLUDES=   -I. -I$(HTSDIR)

prefix      = /usr/local
exec_prefix = $(prefix)
bindir      = $(exec_prefix)/bin
mandir      = $(prefix)/share/man
man1dir     = $(mandir)/man1

INSTALL = install -p
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA    = $(INSTALL) -m 644

# See htslib/Makefile
PACKAGE_VERSION  = 0.0.1
ifneq "$(wildcard .git)" ""
PACKAGE_VERSION := $(shell git describe --always --dirty)
version.h: $(if $(wildcard version.h),$(if $(findstring "$(PACKAGE_VERSION)",$(shell cat version.h)),,force))
endif
version.h:
	echo '#define VERSION "$(PACKAGE_VERSION)"' > $@


.SUFFIXES:.c .o
.PHONY:all install lib test force

force:

.c.o:
		$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@

common.o: common.h common.c version.h $(HTSDIR)/version.h 
tabix.o: common.h tabix.c $(HTSDIR)/htslib/hts.h $(HTSDIR)/htslib/tbx.h $(HTSDIR)/htslib/sam.h $(HTSDIR)/htslib/vcf.h $(HTSDIR)/htslib/kseq.h $(HTSDIR)/htslib/bgzf.h
bgzip.o: common.h bgzip.c  $(HTSDIR)/htslib/bgzf.h

tabix: $(HTSLIB) tabix.o common.o
		$(CC) $(CFLAGS) -o $@ $^ $(HTSLIB) -lpthread -lz -lm -ldl

bgzip: $(HTSLIB) bgzip.o common.o
		$(CC) $(CFLAGS) -o $@ $^ $(HTSLIB) -lpthread -lz -lm -ldl


install: $(PROG)
		mkdir -p $(DESTDIR)$(bindir) $(DESTDIR)$(man1dir)
		$(INSTALL_PROGRAM) $(PROG) plot-vcfstats $(DESTDIR)$(bindir)
		$(INSTALL_DATA) bcftools.1 $(DESTDIR)$(man1dir)

cleanlocal:
		rm -fr gmon.out *.o a.out *.dSYM *~ $(PROG)

clean:cleanlocal
