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
#define MAX_REGIONLINE_LEN 10000
#define MAX_FILENAME 10000

#define error(...) { fprintf(stderr,__VA_ARGS__); return -1; }

int reheader_file(const char *header, const char *file, int meta)
{
    BGZF *fp = bgzf_open(file,"r");
    if (bgzf_read_block(fp) != 0 || !fp->block_length)
        return -1;

    char *buffer = fp->uncompressed_block;
    int skip_until = 0;

    if ( buffer[0]==meta )
    {
        skip_until = 1;

        // Skip the header
        while (1)
        {
            if ( buffer[skip_until]=='\n' )
            {
                skip_until++;
                if ( skip_until>=fp->block_length )
                {
                    if (bgzf_read_block(fp) != 0 || !fp->block_length)
                        error("no body?\n");
                    skip_until = 0;
                }
                // The header has finished
                if ( buffer[skip_until]!=meta ) break;
            }
            skip_until++;
            if ( skip_until>=fp->block_length )
            {
                if (bgzf_read_block(fp) != 0 || !fp->block_length)
                    error("no body?\n");
                skip_until = 0;
            }
        }
    }

    FILE *fh = fopen(header,"r");
    if ( !fh )
        error("%s: %s", header,strerror(errno));
    int page_size = getpagesize();
    char *buf = valloc(page_size);
    BGZF *bgzf_out = bgzf_dopen(fileno(stdout), "w");
    ssize_t nread;
    while ( (nread=fread(buf,1,page_size-1,fh))>0 )
    {
        if ( nread<page_size-1 && buf[nread-1]!='\n' )
            buf[nread++] = '\n';
        if (bgzf_write(bgzf_out, buf, nread) < 0) error("Error: %d\n",bgzf_out->errcode);
    }
    fclose(fh);

    if ( fp->block_length - skip_until > 0 )
    {
        if (bgzf_write(bgzf_out, buffer+skip_until, fp->block_length-skip_until) < 0)
            error("Error: %d\n",fp->errcode);
    }
    if (bgzf_flush(bgzf_out) < 0)
        error("Error: %d\n",bgzf_out->errcode);

    while (1)
    {
        #ifdef _USE_KNETFILE
            nread = knet_read(fp->fp, buf, page_size);
        #else
            nread = fread(buf, 1, page_size, fp->fp);
        #endif
        if ( nread<=0 )
            break;
    
        int count = fwrite(buf, 1, nread, bgzf_out->fp);
        if (count != nread)
            error("Write failed, wrote %d instead of %d bytes.\n", count,(int)nread);
    }

    if (bgzf_close(bgzf_out) < 0)
        error("Error: %d\n",bgzf_out->errcode);

    return 0;
}


int main(int argc, char *argv[])
{
    int c, skip = -1, meta = -1, list_chrms = 0, force = 0, print_header = 0, print_only_header = 0, region_file = 0;
    ti_conf_t conf = ti_conf_gff, *conf_ptr = NULL;
    const char *reheader = NULL;
    char delimiter = 0;
    char line[MAX_REGIONLINE_LEN];

    while ((c = getopt(argc, argv, "Lp:s:b:e:0S:c:lhHfr:d:u:v:T")) >= 0) {
        switch (c) {
            case 'L': region_file=1; break;
            case '0': conf.preset |= TI_FLAG_UCSC; break;
            case 'S': skip = atoi(optarg); break;
            case 'c': meta = optarg[0]; break;
            case 'p':
                if (strcmp(optarg, "gff") == 0) conf_ptr = &ti_conf_gff;
                else if (strcmp(optarg, "bed") == 0) conf_ptr = &ti_conf_bed;
                else if (strcmp(optarg, "sam") == 0) conf_ptr = &ti_conf_sam;
                else if (strcmp(optarg, "vcf") == 0 || strcmp(optarg, "vcf4") == 0) conf_ptr = &ti_conf_vcf;
                else if (strcmp(optarg, "psltbl") == 0) conf_ptr = &ti_conf_psltbl;
                else {
                    fprintf(stderr, "[main] unrecognized preset '%s'\n", optarg);
                    return 1;
                }
                break;
            case 's': conf.sc = atoi(optarg); break;
            case 'd': conf.sc2 = atoi(optarg); break;
            case 'b': conf.bc = atoi(optarg); break;
            case 'e': conf.ec = atoi(optarg); break;
            case 'u': conf.bc2 = atoi(optarg); break;
            case 'v': conf.ec2 = atoi(optarg); break;
            case 'T': delimiter=' '; break;
            case 'l': list_chrms = 1; break;
            case 'h': print_header = 1; break;
            case 'H': print_only_header = 1; break;
            case 'f': force = 1; break;
            case 'r': reheader = optarg; break;
        }
    }
    if(conf.bc2 && !conf.ec2) conf.ec2=conf.bc2;
    if (optind == argc) {
        fprintf(stderr, "\n");
        fprintf(stderr, "Program: pairix (PAIRs file InderXer)\n");
        fprintf(stderr, "Version: %s\n\n", PACKAGE_VERSION);
        fprintf(stderr, "Usage:   pairix <in.pairs.gz> [region1 [region2 [...]]]\n\n");
        fprintf(stderr, "Options: -p STR     preset: gff, bed, sam, vcf, psltbl [gff]\n");
        fprintf(stderr, "         -s INT     sequence name column [1]\n");
        fprintf(stderr, "         -d INT     second sequence name column [null]\n");
        fprintf(stderr, "         -b INT     start1 column [4]\n");
        fprintf(stderr, "         -e INT     end1 column; can be identical to '-b' [5]\n");
        fprintf(stderr, "         -u INT     start2 column [null]\n");
        fprintf(stderr, "         -v INT     end2 column; can be identical to '-u' [null or identical to the start2 specified by -u]\n");
        fprintf(stderr, "         -T         delimiter is space instead of tab.\n");
        fprintf(stderr, "         -L         query region is not a string but a file listing query regions\n");
        fprintf(stderr, "         -S INT     skip first INT lines [0]\n");
        fprintf(stderr, "         -c CHAR    symbol for comment/meta lines [#]\n");
        fprintf(stderr, "         -r FILE    replace the header with the content of FILE [null]\n");
        fprintf(stderr, "         -0         zero-based coordinate\n");
        fprintf(stderr, "         -h         print also the header lines\n");
        fprintf(stderr, "         -H         print only the header lines\n");
        fprintf(stderr, "         -l         list chromosome names\n");
        fprintf(stderr, "         -f         force to overwrite the index\n");
        fprintf(stderr, "\n");
        return 1;
    }
    if ( !conf_ptr )
    {
        int l = strlen(argv[optind]);
        int strcasecmp(const char *s1, const char *s2);
        if (l>=7 && strcasecmp(argv[optind]+l-7, ".gff.gz") == 0) conf_ptr = &ti_conf_gff;
        else if (l>=7 && strcasecmp(argv[optind]+l-7, ".bed.gz") == 0) conf_ptr = &ti_conf_bed;
        else if (l>=7 && strcasecmp(argv[optind]+l-7, ".sam.gz") == 0) conf_ptr = &ti_conf_sam;
        else if (l>=7 && strcasecmp(argv[optind]+l-7, ".vcf.gz") == 0) conf_ptr = &ti_conf_vcf;
        else if (l>=10 && strcasecmp(argv[optind]+l-10, ".psltbl.gz") == 0) conf_ptr = &ti_conf_psltbl;
    }
    if ( conf_ptr )
        conf = *conf_ptr;

    if (skip >= 0) conf.line_skip = skip;
    if (meta >= 0) conf.meta_char = meta;
    if (delimiter) conf.delimiter = delimiter;
    if (list_chrms) {
        ti_index_t *idx;
        int i, n;
        const char **names;
        idx = ti_index_load(argv[optind]);
        if (idx == 0) {
            fprintf(stderr, "[main] fail to load the index file.\n");
            return 1;
        }
        names = ti_seqname(idx, &n);
        for (i = 0; i < n; ++i) printf("%s\n", names[i]);
        free(names);
        ti_index_destroy(idx);
        return 0;
    }
    if (reheader)
        return reheader_file(reheader,argv[optind],conf.meta_char);

    struct stat stat_px2,stat_vcf;
    char *fnidx = calloc(strlen(argv[optind]) + 5, 1);
    strcat(strcpy(fnidx, argv[optind]), ".px2");

    if (optind + 1 == argc && !print_only_header) {
        if (force == 0) {
            if (stat(fnidx, &stat_px2) == 0){
                // Before complaining, check if the VCF file isn't newer. This is a common source of errors,
                // people tend not to notice that tabix failed
                stat(argv[optind], &stat_vcf);
                if ( stat_vcf.st_mtime <= stat_px2.st_mtime )
                {
                    fprintf(stderr, "[tabix] the index file exists. Please use '-f' to overwrite.\n");
                    free(fnidx);
                    return 1;
                }
            }
        }
        if ( bgzf_is_bgzf(argv[optind])!=1 )
        {
            fprintf(stderr,"[tabix] was bgzip used to compress this file? %s\n", argv[optind]);
            free(fnidx);
            return 1;
        }
        if ( !conf_ptr )
        {
            // Building the index but the file type was neither recognised nor given. If no custom change
            //  has been made, warn the user that GFF is used
            if ( conf.preset==ti_conf_gff.preset
                && conf.sc==ti_conf_gff.sc
                && conf.bc==ti_conf_gff.bc
                && conf.ec==ti_conf_gff.ec
                && conf.meta_char==ti_conf_gff.meta_char
                && conf.line_skip==ti_conf_gff.line_skip )
                fprintf(stderr,"[tabix] The file type not recognised and -p not given, using the preset [gff].\n");
        }
        return ti_index_build(argv[optind], &conf);
    }
    { // retrieve
        pairix_t *t;
        // On some systems, stat on non-existent files returns undefined value for sm_mtime, the user had to use -f
        int is_remote = (strstr(fnidx, "ftp://") == fnidx || strstr(fnidx, "http://") == fnidx) ? 1 : 0;
        if ( !is_remote )
        {
            // Common source of errors: new VCF is used with an old index
            stat(fnidx, &stat_px2);
            stat(argv[optind], &stat_vcf);
            if ( force==0 && stat_vcf.st_mtime > stat_px2.st_mtime )
            {
                fprintf(stderr, "[tabix] the index file either does not exist or is older than the vcf file. Please reindex.\n");
                free(fnidx);
                return 1;
            }
        }
        free(fnidx);

        if ((t = ti_open(argv[optind], 0)) == 0) {
            fprintf(stderr, "[main] fail to open the data file.\n");
            return 1;
        }
        if ( print_only_header )
        {
            ti_iter_t iter;
            const char *s;
            int len;
            if (ti_lazy_index_load(t) < 0) {
                fprintf(stderr,"[tabix] failed to load the index file.\n");
                return 1;
            }
            const ti_conf_t *idxconf = ti_get_conf(t->idx);
            iter = ti_query(t, 0, 0, 0);
            while ((s = ti_read(t, iter, &len)) != 0) {
                if ((int)(*s) != idxconf->meta_char) break;
                fputs(s, stdout); fputc('\n', stdout);
            }
            ti_iter_destroy(iter);
            return 0;
        }

        if (argc == optind+2 && strcmp(argv[optind+1], ".") == 0) { // retrieve all
            ti_iter_t iter;
            const char *s;
            int len;
            iter = ti_query(t, 0, 0, 0);
            while ((s = ti_read(t, iter, &len)) != 0) {
                fputs(s, stdout); fputc('\n', stdout);
            }
            ti_iter_destroy(iter);
        } else { // retrieve from specified regions
            int i, len;
            ti_iter_t iter;
            const char *s;
            const ti_conf_t *idxconf;
            
            if (ti_lazy_index_load(t) < 0) {
                fprintf(stderr,"[tabix] failed to load the index file.\n");
                return 1;
            }
            idxconf = ti_get_conf(t->idx);

            if ( print_header )
            {
                // If requested, print the header lines here
                iter = ti_query(t, 0, 0, 0);
                while ((s = ti_read(t, iter, &len)) != 0) {
                    if ((int)(*s) != idxconf->meta_char) break;
                    fputs(s, stdout); fputc('\n', stdout);
                }
                ti_iter_destroy(iter);
                bgzf_seek(t->fp, 0, SEEK_SET);
            }

            if (region_file) {
                for (i = optind + 1; i < argc; ++i) {
                    FILE *FH = fopen(argv[i],"r");
                    if(!FH) { fprintf(stderr, "[main] Can't open region file %s\n", argv[i]); return(1); }
                    while( fgets(line, MAX_REGIONLINE_LEN, FH)) {
                        line[strlen(line)-1]=0; // trim '\n'
                        sequential_iter_t *siter = ti_querys_2d_general(t,line);
                        while((s = sequential_ti_read(siter, &len)) != 0){
                            fputs(s, stdout); fputc('\n',stdout);
                        }
                        destroy_sequential_iter(siter);
                    }
                    fclose(FH);
                }
            } else {
                for (i = optind + 1; i < argc; ++i) {
                    sequential_iter_t *siter = ti_querys_2d_general(t,argv[i]);
                    while((s = sequential_ti_read(siter, &len)) != 0){
                        fputs(s, stdout); fputc('\n',stdout);
                    }
                    destroy_sequential_iter(siter);
                    /*
                    int tid, beg, end, beg2, end2;
                    if (ti_parse_region2d(t->idx, argv[i], &tid, &beg, &end, &beg2, &end2) == 0) {
                        iter = ti_queryi_2d(t, tid, beg, end, beg2, end2);
                        while ((s = ti_read(t, iter, &len)) != 0){
                           fputs(s, stdout); fputc('\n', stdout);
                        }
                        ti_iter_destroy(iter);
                    }
                    else fprintf(stderr, "[main] invalid region: unknown target name or minus interval.\n");
                    */
                }
            }
        }
        ti_close(t);
    }
    return 0;
}
