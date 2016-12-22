#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include "bgzf.h"
#include "pairix.h"
#include "knetfile.h"

#define PACKAGE_VERSION "0.0.1"
#define FILENAMEMAX 2000

int main(int argc, char *argv[])
{
    int i;
    //BGZF *bzfp;
    //int f_dst;

    if(argc==1){
       fprintf(stderr, "\n");
       fprintf(stderr, "Program: streamer_1d\n");
       fprintf(stderr, "Version: %s\n\n", PACKAGE_VERSION);
       fprintf(stderr, "Resorter (convert a file sorted by chr1-chr2-pos1-pos2 to a stream sorted by chr1-pos1)\n\n");
       fprintf(stderr, "Usage:   streamer_1d in.2d.pairs > out.1d.pairs\n\n");
       fprintf(stderr, "Usage:   streamer_1d in.2d.pairs | bgzip -c  > out.1d.pairs.gz\n\n");
       return(1);
    }

    char *fn;
    fn=malloc(FILENAMEMAX*sizeof(char));
    strcpy(fn,argv[1]);

    // write to stdout bgzip (This is slower, so don't use it)
    //f_dst = fileno(stdout);                
    //bzfp = bgzf_dopen(f_dst, "w");

    // actually write merged pairs to bzfp stdout
    //int res = stream_1d(fn, bzfp);
    int res = stream_1d(fn);

    // close bgzf stream
    //if (bgzf_close(bzfp) < 0) fail(bzfp);
    //if (bgzf_close(bzfp) < 0){  fprintf(stderr,"Error: %d\n",bzfp->errcode); return(1); }

    free(fn);
    return(res);
}


