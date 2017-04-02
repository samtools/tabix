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
    int num_fn = argc -1;
    int i;
    //BGZF *bzfp;
    //int f_dst;

    if(argc==1){
       fprintf(stderr, "\n");
       fprintf(stderr, "Program: pairs_merger\n");
       fprintf(stderr, "Version: %s\n\n", PACKAGE_VERSION);
       //fprintf(stderr, "Usage:   pairs_merger <in1.pairs.gz> <in2.pairs.gz> <in3.pairs.gz> ... > out.pairs.gz\n\n");
       fprintf(stderr, "Usage:   pairs_merger <in1.pairs.gz> <in2.pairs.gz> <in3.pairs.gz> ... | bgzip -c  > out.pairs.gz\n\n");
       return(1);
    }

    char *fn_list[num_fn];
    for(i=0;i<num_fn;i++) {
       fn_list[i]=malloc(FILENAMEMAX*sizeof(char));
       strcpy(fn_list[i],argv[i+1]);
    }

    // write to stdout bgzip
    //f_dst = fileno(stdout);                
    //bzfp = bgzf_dopen(f_dst, "w");

    // actually write merged pairs to bzfp stdout
    //int res = pairs_merger(fn_list, num_fn, bzfp);
    int res = pairs_merger(fn_list, num_fn, NULL);

    // close bgzf stream
    //if (bgzf_close(bzfp) < 0) fail(bzfp);
    //if (bgzf_close(bzfp) < 0){  fprintf(stderr,"Error: %d\n",bzfp->errcode); return(1); }


    for(i=0;i<num_fn;i++) {
       free(fn_list[i]);
    }
    return(res);
}


