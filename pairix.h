/* The MIT License

   Copyright (c) 2009 Genome Research Ltd (GRL), 2010 Broad Institute

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

/* Contact: Heng Li <lh3@live.co.uk> */

#ifndef __TABIDX_H
#define __TABIDX_H

#include <stdint.h>
#include "kstring.h"
#include "bgzf.h"

#define TI_PRESET_GENERIC 0
#define TI_PRESET_SAM     1
#define TI_PRESET_VCF     2

#define TI_FLAG_UCSC      0x10000

typedef int (*ti_fetch_f)(int l, const char *s, void *data);

struct __ti_index_t;
typedef struct __ti_index_t ti_index_t;

struct __ti_iter_t;
typedef struct __ti_iter_t *ti_iter_t;

typedef struct {
	BGZF *fp;
	ti_index_t *idx;
	char *fn, *fnidx;
} pairix_t;

typedef struct {
	int32_t preset;
	int32_t sc, bc, ec; // seq col., beg col. and end col.
	int32_t sc2, bc2, ec2; // seq col., beg col. and end col. for the second coordinate
        char delimiter;  // 1 byte for char, but 3 bytes padded so that integers are aligned by 4 bytes.
	int32_t meta_char, line_skip;
} ti_conf_t;  // size 40.

typedef struct {
	int beg, end;
	int beg2, end2;
	char *ss, *se;
	char *ss2, *se2;
} ti_interval_t;

typedef struct {
        int tid, beg, end, bin, beg2, end2, bin2;
} ti_intv_t;

typedef struct {
    pairix_t *t;
    ti_iter_t iter;
    ti_intv_t *intv;
    int *len;
    char *s;  // This points to iter->str.s. It's redundant but it allows us to flush the string without touching iter->str.s itself.
} iter_unit;

typedef struct {
    iter_unit *iu;
    int n;
    char first;
} merged_iter_t;



extern ti_conf_t ti_conf_gff, ti_conf_bed, ti_conf_psltbl, ti_conf_vcf, ti_conf_sam; // preset

#ifdef __cplusplus
extern "C" {
#endif

	/*******************
	 * High-level APIs *
	 *******************/

        pairix_t *load_from_file(char *fn);
	pairix_t *ti_open(const char *fn, const char *fnidx);
	int ti_lazy_index_load(pairix_t *t);
	void ti_close(pairix_t *t);
	ti_iter_t ti_query(pairix_t *t, const char *name, int beg, int end);
	ti_iter_t ti_queryi(pairix_t *t, int tid, int beg, int end);
	ti_iter_t ti_querys(pairix_t *t, const char *reg);
	ti_iter_t ti_query_2d(pairix_t *t, const char *name, int beg, int end, const char *name2, int beg2, int end2);
	ti_iter_t ti_queryi_2d(pairix_t *t, int tid, int beg, int end, int beg2, int end2);
	ti_iter_t ti_querys_2d(pairix_t *t, const char *reg);
	int ti_query_tid(pairix_t *t, const char *name, int beg, int end);
	int ti_querys_tid(pairix_t *t, const char *reg);
	int ti_query_2d_tid(pairix_t *t, const char *name, int beg, int end, const char *name2, int beg2, int end2);
	int ti_querys_2d_tid(pairix_t *t, const char *reg);
	const char *ti_read(pairix_t *t, ti_iter_t iter, int *len);
        const char *merged_ti_read(merged_iter_t *miter, int *len);
        int pairs_merger(char **fn, int n, BGZF *bzfp);
        void stream_1d(char *fn);

	/* Destroy the iterator */
	void ti_iter_destroy(ti_iter_t iter);
        void destroy_merged_iter(merged_iter_t *miter);

	/* Get the list of sequence names. Each "char*" pointer points to a
	 * internal member of the index, so DO NOT modify the returned
	 * pointer; otherwise the index will be corrupted. The returned
	 * pointer should be freed by a single free() call by the routine
	 * calling this function. The number of sequences is returned at *n. */
	const char **ti_seqname(const ti_index_t *idx, int *n);

	/******************
	 * Low-level APIs *
	 ******************/

	/* Build the index for file <fn>. File <fn>.px2 will be generated
	 * and overwrite the file of the same name. Return -1 on failure. */
	int ti_index_build(const char *fn, const ti_conf_t *conf);

	/* Load the index from file <fn>.px2. If <fn> is a URL and the index
	 * file is not in the working directory, <fn>.px2 will be
	 * downloaded. Return NULL on failure. */
	ti_index_t *ti_index_load(const char *fn);

	ti_index_t *ti_index_load_local(const char *fnidx);

	/* Destroy the index */
	void ti_index_destroy(ti_index_t *idx);

	/* Parse a region like: chr2, chr2:100, chr2:100-200. Return -1 on failure. */
	int ti_parse_region(const ti_index_t *idx, const char *str, int *tid, int *begin, int *end);
	int ti_parse_region2d(const ti_index_t *idx, const char *str, int *tid, int *begin, int *end, int *begin2, int *end2);

	int ti_get_tid(const ti_index_t *idx, const char *name);

	/* Get the iterator pointing to the first record at the current file
	 * position. If the file is just openned, the iterator points to the
	 * first record in the file. */
	ti_iter_t ti_iter_first(void);

	/* Get the iterator pointing to the first record in region tid:beg-end */
	ti_iter_t ti_iter_query(const ti_index_t *idx, int tid, int beg, int end, int beg2, int end2);

	/* Get the data line pointed by the iterator and iterate to the next record. */
	const char *ti_iter_read(BGZF *fp, ti_iter_t iter, int *len, char seqonly);

	const ti_conf_t *ti_get_conf(ti_index_t *idx);
	int ti_get_intv(const ti_conf_t *conf, int len, char *line, ti_interval_t *intv);

        /* create an empty merge_iter_t struct */
        merged_iter_t *create_merged_iter(int n);
 
        /* fill in an existing (allocated) iter_unit struct from an iter struct */
        void create_iter_unit(pairix_t *t, ti_iter_t iter, iter_unit *iu);

        int compare_iter_unit (const void *a, const void *b);
        int strcmp2(const void* a, const void* b);
        char** get_unique_merged_seqname(pairix_t **tbs, int n, int *pn_uniq_seq);
        char **merge_seqlist_to_uniq(char** seq_list, int n_seq_list, int *pn_uniq_seq);

        /* bgzip function */
        void fail(BGZF *fp);


	/*******************
	 * Deprecated APIs *
	 *******************/

	/* The callback version for random access */
	int ti_fetch(BGZF *fp, const ti_index_t *idx, int tid, int beg, int end, void *data, ti_fetch_f func);
	int ti_fetch_2d(BGZF *fp, const ti_index_t *idx, int tid, int beg, int end, int beg2, int end2, void *data, ti_fetch_f func);

	/* Read one line. */
	int ti_readline(BGZF *fp, kstring_t *str);

#ifdef __cplusplus
}
#endif

#endif
