CC=			gcc
CFLAGS=		-g -Wall -O2 -fPIC #-m64 #-arch ppc
DFLAGS=		-D_FILE_OFFSET_BITS=64 -D_USE_KNETFILE -DBGZF_CACHE
LOBJS=		bgzf.o kstring.o knetfile.o index.o bedidx.o
AOBJS=		main.o
PROG=		pairix bgzip pairs_merger merge-pairs
INCLUDES=
SUBDIRS=	.
LIBPATH=
LIBCURSES=

.SUFFIXES:.c .o

.c.o:
		$(CC) -c $(CFLAGS) $(DFLAGS) $(INCLUDES) $< -o $@

all-recur lib-recur clean-recur cleanlocal-recur install-recur:
		@target=`echo $@ | sed s/-recur//`; \
		wdir=`pwd`; \
		list='$(SUBDIRS)'; for subdir in $$list; do \
			cd $$subdir; \
			$(MAKE) CC="$(CC)" DFLAGS="$(DFLAGS)" CFLAGS="$(CFLAGS)" \
				INCLUDES="$(INCLUDES)" LIBPATH="$(LIBPATH)" $$target || exit 1; \
			cd $$wdir; \
		done;

all:$(PROG)
		mkdir -p bin; mv pairix bgzip pairs_merger bin; cp merge-pairs/merge-pairs bin; chmod +x bin/*

lib:libpairix.a

libpairix.so.1:$(LOBJS)
		$(CC) -shared -Wl,-soname,libpairix.so -o $@ $(LOBJS) -lc -lz

libpairix.1.dylib:$(LOBJS)
		libtool -dynamic $(LOBJS) -o $@ -lc -lz

libpairix.a:$(LOBJS)
		$(AR) -csru $@ $(LOBJS)

pairix:lib $(AOBJS)
		$(CC) $(CFLAGS) -o $@ $(AOBJS) -L. -lpairix -lm $(LIBPATH) -lz

bgzip:bgzip.o bgzf.o knetfile.o
		$(CC) $(CFLAGS) -o $@ bgzip.o bgzf.o knetfile.o -lz

pairs_merger:pairs_merger.o lib
		$(CC) $(CFLAGS) -o $@ pairs_merger.o -L. -lpairix -lm $(LIBPATH) -lz

TabixReader.class:TabixReader.java
		javac -cp .:sam.jar TabixReader.java

kstring.o:kstring.h
knetfile.o:knetfile.h
bgzf.o:bgzf.h knetfile.h
index.o:bgzf.h pairix.h khash.h ksort.h kstring.h
main.o:pairix.h kstring.h bgzf.h
pairs_merger.o:pairix.h kstring.h bgzf.h
bgzip.o:bgzf.h
bedidx.o:kseq.h khash.h


cleanlocal:
		rm -fr gmon.out *.o a.out *.dSYM $(PROG) *~ *.a tabix.aux tabix.log *.class libpairix.*.dylib libpairix.so*

clean:cleanlocal-recur
