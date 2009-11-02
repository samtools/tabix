#include <ctype.h>
#include <assert.h>
#include "khash.h"
#include "ksort.h"
#include "kstring.h"
#include "bam_endian.h"
#ifdef _USE_KNETFILE
#include "knetfile.h"
#endif
#include "tabix.h"

#define TAD_MIN_CHUNK_GAP 32768
// 1<<14 is the size of minimum bin.
#define TAD_LIDX_SHIFT    14

typedef struct {
	uint64_t u, v;
} pair64_t;

#define pair64_lt(a,b) ((a).u < (b).u)
KSORT_INIT(off, pair64_t, pair64_lt)

typedef struct {
	uint32_t m, n;
	pair64_t *list;
} ti_binlist_t;

typedef struct {
	int32_t n, m;
	uint64_t *offset;
} ti_lidx_t;

KHASH_MAP_INIT_INT(i, ti_binlist_t)
KHASH_MAP_INIT_STR(s, int)

struct __ti_index_t {
	ti_conf_t conf;
	int32_t n, max;
	khash_t(s) *tname;
	khash_t(i) **index;
	ti_lidx_t *index2;
};

typedef struct {
	int tid, beg, end, bin;
} ti_intv_t;


ti_conf_t ti_conf_gff = { 0, 1, 4, 5 };


int ti_readline(BGZF *fp, kstring_t *str)
{
	int c, l = 0;
	str->l = 0;
	while ((c = bgzf_getc(fp)) >= 0 && c != '\n') {
		++l;
		if (c != '\r') kputc(c, str);
	}
	if (c < 0 && l == 0) return -1; // end of file
	return str->l;
}

// requirement: len <= LEN_MASK
static inline void insert_offset(khash_t(i) *h, int bin, uint64_t beg, uint64_t end)
{
	khint_t k;
	ti_binlist_t *l;
	int ret;
	k = kh_put(i, h, bin, &ret);
	l = &kh_value(h, k);
	if (ret) { // not present
		l->m = 1; l->n = 0;
		l->list = (pair64_t*)calloc(l->m, 16);
	}
	if (l->n == l->m) {
		l->m <<= 1;
		l->list = (pair64_t*)realloc(l->list, l->m * 16);
	}
	l->list[l->n].u = beg; l->list[l->n++].v = end;
}

static inline void insert_offset2(ti_lidx_t *index2, int _beg, int _end, uint64_t offset)
{
	int i, beg, end;
	beg = _beg >> TAD_LIDX_SHIFT;
	end = (_end - 1) >> TAD_LIDX_SHIFT;
	if (index2->m < end + 1) {
		int old_m = index2->m;
		index2->m = end + 1;
		kroundup32(index2->m);
		index2->offset = (uint64_t*)realloc(index2->offset, index2->m * 8);
		memset(index2->offset + old_m, 0, 8 * (index2->m - old_m));
	}
	for (i = beg + 1; i <= end; ++i)
		if (index2->offset[i] == 0) index2->offset[i] = offset;
	index2->n = end + 1;
}

static void merge_chunks(ti_index_t *idx)
{
	khash_t(i) *index;
	int i, l, m;
	khint_t k;
	for (i = 0; i < idx->n; ++i) {
		index = idx->index[i];
		for (k = kh_begin(index); k != kh_end(index); ++k) {
			ti_binlist_t *p;
			if (!kh_exist(index, k)) continue;
			p = &kh_value(index, k);
			m = 0;
			for (l = 1; l < p->n; ++l) {
				if (p->list[m].v>>16 == p->list[l].u>>16) p->list[m].v = p->list[l].v;
				else p->list[++m] = p->list[l];
			} // ~for(l)
			p->n = m + 1;
		} // ~for(k)
	} // ~for(i)
}

static inline int ti_reg2bin(uint32_t beg, uint32_t end)
{
	--end;
	if (beg>>14 == end>>14) return 4681 + (beg>>14);
	if (beg>>17 == end>>17) return  585 + (beg>>17);
	if (beg>>20 == end>>20) return   73 + (beg>>20);
	if (beg>>23 == end>>23) return    9 + (beg>>23);
	if (beg>>26 == end>>26) return    1 + (beg>>26);
	return 0;
}

static int get_intv(ti_index_t *idx, kstring_t *str, ti_intv_t *intv)
{
	khint_t k;
	int i, b = 0, id = 1;
	char *s;

	if (idx->conf.preset != TI_PRESET_GENERIC) return -1;
	intv->tid = intv->beg = intv->end = intv->bin = -1;
	for (i = 0; i <= str->l; ++i) {
		if (str->s[i] == '\t' || str->s[i] == 0) {
			if (id == idx->conf.sc) {
				str->s[i] = 0;
				k = kh_get(s, idx->tname, str->s + b);
				if (k == kh_end(idx->tname)) { // a new target sequence
					int ret, size;
					// update idx->n, ->max, ->index and ->index2
					if (idx->n == idx->max) {
						idx->max = idx->max? idx->max<<1 : 8;
						idx->index = realloc(idx->index, idx->max * sizeof(void*));
						idx->index2 = realloc(idx->index2, idx->max * sizeof(ti_lidx_t));
					}
					memset(&idx->index2[idx->n], 0, sizeof(ti_lidx_t));
					idx->index[idx->n++] = kh_init(i);
					// update ->tname
					intv->tid = size = kh_size(idx->tname);
					s = strdup(str->s + b);
					k = kh_put(s, idx->tname, s, &ret);
					kh_value(idx->tname, k) = size;
					assert(idx->n == kh_size(idx->tname));
				} else intv->tid = kh_value(idx->tname, k);
				if (i != str->l) str->s[i] = '\t';
			} else if (id == idx->conf.bc) {
				intv->beg = strtol(str->s + b, &s, 0);
			} else if (id == idx->conf.ec) {
				intv->end = strtol(str->s + b, &s, 0);
			}
			b = i + 1;
			++id;
		}
	}
	if (intv->tid < 0 || intv->beg < 0 || intv->end < 0) return -1;
	intv->bin = ti_reg2bin(intv->beg-1, intv->end);
	return 0;
}

ti_index_t *ti_index_core(BGZF *fp, const ti_conf_t *conf)
{
	int ret;
	ti_index_t *idx;
	uint32_t last_bin, save_bin;
	int32_t last_coor, last_tid, save_tid;
	uint64_t save_off, last_off;
	kstring_t *str;

	str = calloc(1, sizeof(kstring_t));

	idx = (ti_index_t*)calloc(1, sizeof(ti_index_t));
	idx->conf = *conf;
	idx->n = idx->max = 0;
	idx->tname = kh_init(s);
	idx->index = 0;
	idx->index2 = 0;

	save_bin = save_tid = last_tid = last_bin = 0xffffffffu;
	save_off = last_off = bgzf_tell(fp); last_coor = 0xffffffffu;
	while ((ret = ti_readline(fp, str)) >= 0) {
		ti_intv_t intv;
		get_intv(idx, str, &intv);
		if (last_tid != intv.tid) { // change of chromosomes
			last_tid = intv.tid;
			last_bin = 0xffffffffu;
		} else if (last_coor > intv.beg) {
			fprintf(stderr, "[ti_index_core] the file is not sorted: %u > %u in %d-th chr\n",
					last_coor, intv.beg, intv.tid + 1);
			exit(1);
		}
		if (intv.bin < 4681) insert_offset2(&idx->index2[intv.tid], intv.beg, intv.end, last_off);
		if (intv.bin != last_bin) { // then possibly write the binning index
			if (save_bin != 0xffffffffu) // save_bin==0xffffffffu only happens to the first record
				insert_offset(idx->index[save_tid], save_bin, save_off, last_off);
			save_off = last_off;
			save_bin = last_bin = intv.bin;
			save_tid = intv.tid;
			if (save_tid < 0) break;
		}
		if (bgzf_tell(fp) <= last_off) {
			fprintf(stderr, "[ti_index_core] bug in BGZF: %llx < %llx\n",
					(unsigned long long)bgzf_tell(fp), (unsigned long long)last_off);
			exit(1);
		}
		last_off = bgzf_tell(fp);
		last_coor = intv.beg;
	}
	if (save_tid >= 0) insert_offset(idx->index[save_tid], save_bin, save_off, bgzf_tell(fp));
	merge_chunks(idx);

	free(str->s); free(str);
	return idx;
}

void ti_index_destroy(ti_index_t *idx)
{
	khint_t k;
	int i;
	if (idx == 0) return;
	// destroy the name hash table
	for (k = kh_begin(idx->tname); k != kh_end(idx->tname); ++k) {
		if (kh_exist(idx->tname, k))
			free((char*)kh_key(idx->tname, k));
	}
	kh_destroy(s, idx->tname);
	// destroy the binning index
	for (i = 0; i < idx->n; ++i) {
		khash_t(i) *index = idx->index[i];
		ti_lidx_t *index2 = idx->index2 + i;
		for (k = kh_begin(index); k != kh_end(index); ++k) {
			if (kh_exist(index, k))
				free(kh_value(index, k).list);
		}
		kh_destroy(i, index);
		free(index2->offset);
	}
	free(idx->index);
	// destroy the linear index
	free(idx->index2);
	free(idx);
}

void ti_index_save(const ti_index_t *idx, FILE *fp)
{
	int32_t i, size, ti_is_be;
	khint_t k;
	ti_is_be = bam_is_big_endian();
	fwrite("TAI\1", 1, 4, fp);
	if (ti_is_be) {
		uint32_t x = idx->n;
		fwrite(bam_swap_endian_4p(&x), 4, 1, fp);
	} else fwrite(&idx->n, 4, 1, fp);
	if (ti_is_be) { // write ti_conf_t;
		uint32_t x;
		x = bam_swap_endian_4(idx->conf.preset); fwrite(&x, 4, 1, fp);
		x = bam_swap_endian_4(idx->conf.sc); fwrite(&x, 4, 1, fp);
		x = bam_swap_endian_4(idx->conf.bc); fwrite(&x, 4, 1, fp);
		x = bam_swap_endian_4(idx->conf.ec); fwrite(&x, 4, 1, fp);
	} else fwrite(&idx->conf, sizeof(ti_conf_t), 1, fp);
	{ // write target names
		char **name;
		name = calloc(kh_size(idx->tname), sizeof(void*));
		for (k = kh_begin(idx->tname); k != kh_end(idx->tname); ++k) {
			if (kh_exist(idx->tname, k))
				name[kh_value(idx->tname, k)] = (char*)kh_key(idx->tname, k);
		}
		for (i = 0; i < kh_size(idx->tname); ++i)
			fwrite(name[i], 1, strlen(name[i]) + 1, fp);
		free(name);
	}
	for (i = 0; i < idx->n; ++i) {
		khash_t(i) *index = idx->index[i];
		ti_lidx_t *index2 = idx->index2 + i;
		// write binning index
		size = kh_size(index);
		if (ti_is_be) { // big endian
			uint32_t x = size;
			fwrite(bam_swap_endian_4p(&x), 4, 1, fp);
		} else fwrite(&size, 4, 1, fp);
		for (k = kh_begin(index); k != kh_end(index); ++k) {
			if (kh_exist(index, k)) {
				ti_binlist_t *p = &kh_value(index, k);
				if (ti_is_be) { // big endian
					uint32_t x;
					x = kh_key(index, k); fwrite(bam_swap_endian_4p(&x), 4, 1, fp);
					x = p->n; fwrite(bam_swap_endian_4p(&x), 4, 1, fp);
					for (x = 0; (int)x < p->n; ++x) {
						bam_swap_endian_8p(&p->list[x].u);
						bam_swap_endian_8p(&p->list[x].v);
					}
					fwrite(p->list, 16, p->n, fp);
					for (x = 0; (int)x < p->n; ++x) {
						bam_swap_endian_8p(&p->list[x].u);
						bam_swap_endian_8p(&p->list[x].v);
					}
				} else {
					fwrite(&kh_key(index, k), 4, 1, fp);
					fwrite(&p->n, 4, 1, fp);
					fwrite(p->list, 16, p->n, fp);
				}
			}
		}
		// write linear index (index2)
		if (ti_is_be) {
			int x = index2->n;
			fwrite(bam_swap_endian_4p(&x), 4, 1, fp);
		} else fwrite(&index2->n, 4, 1, fp);
		if (ti_is_be) { // big endian
			int x;
			for (x = 0; (int)x < index2->n; ++x)
				bam_swap_endian_8p(&index2->offset[x]);
			fwrite(index2->offset, 8, index2->n, fp);
			for (x = 0; (int)x < index2->n; ++x)
				bam_swap_endian_8p(&index2->offset[x]);
		} else fwrite(index2->offset, 8, index2->n, fp);
	}
	fflush(fp);
}

static ti_index_t *ti_index_load_core(FILE *fp)
{
	int i, ti_is_be;
	char magic[4];
	ti_index_t *idx;
	ti_is_be = bam_is_big_endian();
	if (fp == 0) {
		fprintf(stderr, "[ti_index_load_core] fail to load index.\n");
		return 0;
	}
	fread(magic, 1, 4, fp);
	if (strncmp(magic, "TAI\1", 4)) {
		fprintf(stderr, "[ti_index_load] wrong magic number.\n");
		fclose(fp);
		return 0;
	}
	idx = (ti_index_t*)calloc(1, sizeof(ti_index_t));	
	fread(&idx->n, 4, 1, fp);
	if (ti_is_be) bam_swap_endian_4p(&idx->n);
	idx->tname = kh_init(s);
	idx->index = (khash_t(i)**)calloc(idx->n, sizeof(void*));
	idx->index2 = (ti_lidx_t*)calloc(idx->n, sizeof(ti_lidx_t));
	// read idx->conf
	fread(&idx->conf, sizeof(ti_conf_t), 1, fp);
	if (ti_is_be) {
		bam_swap_endian_4p(&idx->conf.preset);
		bam_swap_endian_4p(&idx->conf.sc);
		bam_swap_endian_4p(&idx->conf.bc);
		bam_swap_endian_4p(&idx->conf.ec);
	}
	{ // read target names
		int c, j, ret;
		kstring_t *str;
		khint_t k;
		str = calloc(1, sizeof(kstring_t));
		j = 0;
		while ((c = fgetc(fp)) != EOF) {
			if (c == 0) {
				k = kh_put(s, idx->tname, strdup(str->s), &ret);
				kh_value(idx->tname, k) = j++;
				str->l = 0;
				if (j == idx->n) break;
			} else kputc(c, str);
		}
		free(str->s); free(str);
	}
	for (i = 0; i < idx->n; ++i) {
		khash_t(i) *index;
		ti_lidx_t *index2 = idx->index2 + i;
		uint32_t key, size;
		khint_t k;
		int j, ret;
		ti_binlist_t *p;
		index = idx->index[i] = kh_init(i);
		// load binning index
		fread(&size, 4, 1, fp);
		if (ti_is_be) bam_swap_endian_4p(&size);
		for (j = 0; j < (int)size; ++j) {
			fread(&key, 4, 1, fp);
			if (ti_is_be) bam_swap_endian_4p(&key);
			k = kh_put(i, index, key, &ret);
			p = &kh_value(index, k);
			fread(&p->n, 4, 1, fp);
			if (ti_is_be) bam_swap_endian_4p(&p->n);
			p->m = p->n;
			p->list = (pair64_t*)malloc(p->m * 16);
			fread(p->list, 16, p->n, fp);
			if (ti_is_be) {
				int x;
				for (x = 0; x < p->n; ++x) {
					bam_swap_endian_8p(&p->list[x].u);
					bam_swap_endian_8p(&p->list[x].v);
				}
			}
		}
		// load linear index
		fread(&index2->n, 4, 1, fp);
		if (ti_is_be) bam_swap_endian_4p(&index2->n);
		index2->m = index2->n;
		index2->offset = (uint64_t*)calloc(index2->m, 8);
		fread(index2->offset, index2->n, 8, fp);
		if (ti_is_be)
			for (j = 0; j < index2->n; ++j) bam_swap_endian_8p(&index2->offset[j]);
	}
	return idx;
}

ti_index_t *ti_index_load_local(const char *_fn)
{
	FILE *fp;
	char *fnidx, *fn;

	if (strstr(_fn, "ftp://") == _fn || strstr(_fn, "http://") == _fn) {
		const char *p;
		int l = strlen(_fn);
		for (p = _fn + l - 1; p >= _fn; --p)
			if (*p == '/') break;
		fn = strdup(p + 1);
	} else fn = strdup(_fn);
	fnidx = (char*)calloc(strlen(fn) + 5, 1);
	strcpy(fnidx, fn); strcat(fnidx, ".idx");
	fp = fopen(fnidx, "r");
	free(fnidx); free(fn);
	if (fp) {
		ti_index_t *idx = ti_index_load_core(fp);
		fclose(fp);
		return idx;
	} else return 0;
}

#ifdef _USE_KNETFILE
static void download_from_remote(const char *url)
{
	const int buf_size = 1 * 1024 * 1024;
	char *fn;
	FILE *fp;
	uint8_t *buf;
	knetFile *fp_remote;
	int l;
	if (strstr(url, "ftp://") != url && strstr(url, "http://") != url) return;
	l = strlen(url);
	for (fn = (char*)url + l - 1; fn >= url; --fn)
		if (*fn == '/') break;
	++fn; // fn now points to the file name
	fp_remote = knet_open(url, "r");
	if (fp_remote == 0) {
		fprintf(stderr, "[download_from_remote] fail to open remote file.\n");
		return;
	}
	if ((fp = fopen(fn, "w")) == 0) {
		fprintf(stderr, "[download_from_remote] fail to create file in the working directory.\n");
		knet_close(fp_remote);
		return;
	}
	buf = (uint8_t*)calloc(buf_size, 1);
	while ((l = knet_read(fp_remote, buf, buf_size)) != 0)
		fwrite(buf, 1, l, fp);
	free(buf);
	fclose(fp);
	knet_close(fp_remote);
}
#else
static void download_from_remote(const char *url)
{
	return;
}
#endif

ti_index_t *ti_index_load(const char *fn)
{
	ti_index_t *idx;
	idx = ti_index_load_local(fn);
	if (idx == 0 && (strstr(fn, "ftp://") == fn || strstr(fn, "http://") == fn)) {
		char *fnidx = calloc(strlen(fn) + 5, 1);
		strcat(strcpy(fnidx, fn), ".idx");
		fprintf(stderr, "[ti_index_load] attempting to download the remote index file.\n");
		download_from_remote(fnidx);
		idx = ti_index_load_local(fn);
	}
	if (idx == 0) fprintf(stderr, "[ti_index_load] fail to load BAM index.\n");
	return idx;
}

int ti_index_build2(const char *fn, const ti_conf_t *conf, const char *_fnidx)
{
	char *fnidx;
	FILE *fpidx;
	BGZF *fp;
	ti_index_t *idx;
	if ((fp = bgzf_open(fn, "r")) == 0) {
		fprintf(stderr, "[ti_index_build2] fail to open the BAM file.\n");
		return -1;
	}
	idx = ti_index_core(fp, conf);
	bgzf_close(fp);
	if (_fnidx == 0) {
		fnidx = (char*)calloc(strlen(fn) + 5, 1);
		strcpy(fnidx, fn); strcat(fnidx, ".idx");
	} else fnidx = strdup(_fnidx);
	fpidx = fopen(fnidx, "w");
	if (fpidx == 0) {
		fprintf(stderr, "[ti_index_build2] fail to create the index file.\n");
		free(fnidx);
		return -1;
	}
	ti_index_save(idx, fpidx);
	ti_index_destroy(idx);
	fclose(fpidx);
	free(fnidx);
	return 0;
}

int ti_index_build(const char *fn, const ti_conf_t *conf)
{
	return ti_index_build2(fn, conf, 0);
}

/********************************************
 * parse a region in the format chr:beg-end *
 ********************************************/

int ti_parse_region(ti_index_t *idx, const char *str, int *tid, int *begin, int *end)
{
	char *s, *p;
	int i, l, k;
	khiter_t iter;
	khash_t(s) *h = idx->tname;
	l = strlen(str);
	p = s = (char*)malloc(l+1);
	/* squeeze out "," */
	for (i = k = 0; i != l; ++i)
		if (str[i] != ',' && !isspace(str[i])) s[k++] = str[i];
	s[k] = 0;
	for (i = 0; i != k; ++i) if (s[i] == ':') break;
	s[i] = 0;
	iter = kh_get(s, h, s); /* get the tid */
	if (iter == kh_end(h)) { // name not found
		*tid = -1; free(s);
		return -1;
	}
	*tid = kh_value(h, iter);
	if (i == k) { /* dump the whole sequence */
		*begin = 0; *end = 1<<29; free(s);
		return 0;
	}
	for (p = s + i + 1; i != k; ++i) if (s[i] == '-') break;
	*begin = atoi(p);
	if (i < k) {
		p = s + i + 1;
		*end = atoi(p);
	} else *end = 1<<29;
	if (*begin > 0) --*begin;
	free(s);
	if (*begin > *end) return -1;
	return 0;
}

/*******************************
 * retrieve a specified region *
 *******************************/

#define MAX_BIN 37450 // =(8^6-1)/7+1

static inline int reg2bins(uint32_t beg, uint32_t end, uint16_t list[MAX_BIN])
{
	int i = 0, k;
	--end;
	list[i++] = 0;
	for (k =    1 + (beg>>26); k <=    1 + (end>>26); ++k) list[i++] = k;
	for (k =    9 + (beg>>23); k <=    9 + (end>>23); ++k) list[i++] = k;
	for (k =   73 + (beg>>20); k <=   73 + (end>>20); ++k) list[i++] = k;
	for (k =  585 + (beg>>17); k <=  585 + (end>>17); ++k) list[i++] = k;
	for (k = 4681 + (beg>>14); k <= 4681 + (end>>14); ++k) list[i++] = k;
	return i;
}

static inline int is_overlap(int32_t beg, int32_t end, int rbeg, int rend)
{
	return (rend > beg && rbeg < end);
}

// ti_fetch helper function retrieves 
pair64_t *get_chunk_coordinates(const ti_index_t *idx, int tid, int beg, int end, int* cnt_off)
{
	uint16_t *bins;
	int i, n_bins, n_off;
	pair64_t *off;
	khint_t k;
	khash_t(i) *index;
	uint64_t min_off;

	bins = (uint16_t*)calloc(MAX_BIN, 2);
	n_bins = reg2bins(beg, end, bins);
	index = idx->index[tid];
	min_off = (beg>>TAD_LIDX_SHIFT >= idx->index2[tid].n)? 0 : idx->index2[tid].offset[beg>>TAD_LIDX_SHIFT];
	for (i = n_off = 0; i < n_bins; ++i) {
		if ((k = kh_get(i, index, bins[i])) != kh_end(index))
			n_off += kh_value(index, k).n;
	}
	if (n_off == 0) {
		free(bins); return 0;
	}
	off = (pair64_t*)calloc(n_off, 16);
	for (i = n_off = 0; i < n_bins; ++i) {
		if ((k = kh_get(i, index, bins[i])) != kh_end(index)) {
			int j;
			ti_binlist_t *p = &kh_value(index, k);
			for (j = 0; j < p->n; ++j)
				if (p->list[j].v > min_off) off[n_off++] = p->list[j];
		}
	}
	free(bins);
	{
		int l;
		ks_introsort(off, n_off, off);
		// resolve completely contained adjacent blocks
		for (i = 1, l = 0; i < n_off; ++i)
			if (off[l].v < off[i].v)
				off[++l] = off[i];
		n_off = l + 1;
		// resolve overlaps between adjacent blocks; this may happen due to the merge in indexing
		for (i = 1; i < n_off; ++i)
			if (off[i-1].v >= off[i].u) off[i-1].v = off[i].u;
		{ // merge adjacent blocks
			for (i = 1, l = 0; i < n_off; ++i) {
				if (off[l].v>>16 == off[i].u>>16) off[l].v = off[i].v;
				else off[++l] = off[i];
			}
			n_off = l + 1;
		}
	}
	*cnt_off = n_off;
	return off;
}

int ti_fetch(BGZF *fp, const ti_index_t *idx, int tid, int beg, int end, void *data, ti_fetch_f func)
{
	int n_off;
	pair64_t *off = get_chunk_coordinates(idx, tid, beg, end, &n_off);
	if (off == 0) return 0;
	{
		// retrive alignments
		uint64_t curr_off;
		int i, ret, n_seeks;
		kstring_t *str = calloc(1, sizeof(kstring_t));
		n_seeks = 0; i = -1; curr_off = 0;
		for (;;) {
			if (curr_off == 0 || curr_off >= off[i].v) { // then jump to the next chunk
				if (i == n_off - 1) break; // no more chunks
				if (i >= 0) assert(curr_off == off[i].v); // otherwise bug
				if (i < 0 || off[i].v != off[i+1].u) { // not adjacent chunks; then seek
					bgzf_seek(fp, off[i+1].u, SEEK_SET);
					curr_off = bgzf_tell(fp);
					++n_seeks;
				}
				++i;
			}
			if ((ret = ti_readline(fp, str)) >= 0) {
				ti_intv_t intv;
				get_intv((ti_index_t*)idx, str, &intv);
				curr_off = bgzf_tell(fp);
				if (intv.tid != tid || intv.end >= end) break; // no need to proceed
				else if (is_overlap(beg, end, intv.beg, intv.end)) func(str->l, str->s, data);
			} else break; // end of file
		}
//		fprintf(stderr, "[ti_fetch] # seek calls: %d\n", n_seeks);
		free(str->s); free(str);
	}
	free(off);
	return 0;
}
