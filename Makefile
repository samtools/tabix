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
LIBSUFFIX=dylib
LIBNAME=libtabix.$(SOVERSION).$(LIBSUFFIX)
CFLAGS +=
else
LIBSUFFIX=so
LIBNAME=libtabix.$(LIBSUFFIX).$(SOVERSION)
CFLAGS +=
endif

# Set architecture flags
ARCH := $(shell uname -m)
ifeq ($(ARCH), x86_64)
CFLAGS += -m64 -fPIC
#else ifeq ($(ARCH), ppc -m?)
endif

ifdef SHARED
CFLAGS += -fPIC
else #static
LIBSUFFIX=a
LIBNAME=libtabix.$(LIBSUFFIX)
endif

.SUFFIXES:.c .o

.c.o:
		$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@

all:$(PROG)

libtabix.$(LIBSUFFIX): $(LIBNAME)
		$(LN) -s $< $@

libtabix.so.$(SOVERSION):$(LOBJS)
		$(CC) -shared -Wl,-soname,libtabix.so -o $@ $(LOBJS) -lc -lz

libtabix.$(SOVERSION).dylib:$(LOBJS)
		libtool -dynamic $(LOBJS) -o $@ -lc -lz

libtabix.a:$(LOBJS)
		$(AR) -csru $@ $(LOBJS)

tabix:$(AOBJS) libtabix.$(LIBSUFFIX)
		$(CC) $(CFLAGS) -o $@ $(AOBJS) -L. -ltabix -lm $(LIBPATH) -lz

bgzip:$(BGZOBJS) libtabix.$(LIBSUFFIX)
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
		rm -fr gmon.out *.o a.out *.dSYM $(PROG) *~ *.a tabix.aux tabix.log tabix.pdf *.class libtabix.*.dylib libtabix.so*
