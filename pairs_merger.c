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

    if(argc==1){
       fprintf(stderr, "\n");
       fprintf(stderr, "Program: pairs_merger\n");
       fprintf(stderr, "Version: %s\n\n", PACKAGE_VERSION);
       fprintf(stderr, "Usage:   pairs_merger <in1.pairs.gz> <in2.pairs.gz> <in2.pairs.gz> > out.pairs\n\n");
       return(1);
    }

    char *fn_list[num_fn];
    for(i=0;i<num_fn;i++) {
       fn_list[i]=malloc(FILENAMEMAX*sizeof(char));
       strcpy(fn_list[i],argv[i+1]);
    }
    int res = pairs_merger(fn_list,num_fn);

    return(res);
}

