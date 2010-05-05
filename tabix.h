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

typedef struct {
	int32_t preset;
	int32_t sc, bc, ec;
	int32_t meta_char, line_skip;
} ti_conf_t;

extern ti_conf_t ti_conf_gff, ti_conf_bed, ti_conf_psltbl, ti_conf_vcf, ti_conf_sam;

#ifdef __cplusplus
extern "C" {
#endif

	int ti_readline(BGZF *fp, kstring_t *str);

	int ti_index_build(const char *fn, const ti_conf_t *conf);
	ti_index_t *ti_index_load(const char *fn);
    int ti_list_chromosomes(const char *fn);
	void ti_index_destroy(ti_index_t *idx);
	int ti_parse_region(ti_index_t *idx, const char *str, int *tid, int *begin, int *end);
	int ti_fetch(BGZF *fp, const ti_index_t *idx, int tid, int beg, int end, void *data, ti_fetch_f func);

#ifdef __cplusplus
}
#endif

#endif
