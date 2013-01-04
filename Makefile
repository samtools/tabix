CC=		gcc
LN=		ln
CFLAGS=		-g -Wall -O2
DFLAGS=		-D_FILE_OFFSET_BITS=64 -D_USE_KNETFILE -DBGZF_CACHE
KLIBOBJ=	bgzf.o kstring.o knetfile.o
LOBJS=		$(KLIBOBJ) index.o bedidx.o
BGZOBJS=	bgzip.o bgzf.o
AOBJS=		main.o
PROG=		tabix bgzip
INCLUDES=
SUBDIRS=	.
LIBPATH=
LIBCURSES=
SOVERSION=1

# Pick os
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
SOSUFFIX=dylib
SONAME=libtabix.$(SOVERSION).$(SOSUFFIX)
SOFLAGS=-dynamic
LINK=libtool
CFLAGS +=
else
SOSUFFIX=so
SONAME=libtabix.$(SOSUFFIX).$(SOVERSION)
SOFLAGS=-shared -Wl,-soname,libtabix.$(SOSUFFIX)
CFLAGS +=
LINK=gcc
endif

# Set architecture flags
ARCH := $(shell uname -m)
ifeq ($(ARCH), x86_64)
CFLAGS += -m64 -fPIC
#else ifeq ($(ARCH), ppc -m?)
endif

ifdef SHARED
CFLAGS += -fPIC
LIBNAME=libtabix.$(SOSUFFIX)
else #static
LIBNAME=libtabix.a
endif

.SUFFIXES:.c .o

.c.o:
		$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@

all:$(PROG)

$(SONAME): $(LOBJS)
		$(LINK) $(SOFLAGS) -o $@ $(LOBJS) -lc -lz

libtabix.$(SOSUFFIX): $(SONAME)
		$(LN) -s $^ $@

libtabix.a:$(LOBJS)
		$(AR) -csru $@ $(LOBJS)

tabix:$(AOBJS) $(LIBNAME)
		$(CC) $(CFLAGS) -o $@ $(AOBJS) -L. -ltabix -lm $(LIBPATH) -lz

bgzip:$(BGZOBJS) $(LIBNAME)
		$(CC) $(CFLAGS) -o $@ $(BGZOBJS) -L. -ltabix -lz

TabixReader.class:TabixReader.java
		javac -cp .:sam.jar TabixReader.java

kstring.o:kstring.h
knetfile.o:knetfile.h
bgzf.o:bgzf.h knetfile.h
index.o:bgzf.h tabix.h khash.h ksort.h kstring.h
main.o:tabix.h kstring.h bgzf.h
bgzip.o:bgzf.h
bedidx.o:kseq.h khash.h

tabix.pdf:tabix.tex
		pdflatex tabix.tex

clean:
		rm -fr gmon.out *.o a.out *.dSYM $(PROG) *~ *.a tabix.aux tabix.log tabix.pdf *.class $(SONAME) *.$(SOSUFFIX)
