#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <stdlib.h>
#include "bgzf.h"
#include "tabix.h"

typedef struct {
	BGZF *fp;
	ti_index_t *idx;
} tabix_t;

tabix_t *_tabix_open(const char *fn)
{
	BGZF *fp;
	ti_index_t *idx;
	tabix_t *t;
	if ((fp = bgzf_open(fn, "r")) == 0) return 0;
	if ((idx = ti_index_load(fn)) == 0) return 0;
	t = calloc(1, sizeof(tabix_t));
	t->fp = fp; t->idx = idx;
	return t;
}

void _tabix_close(tabix_t *t)
{
	if (t) {
	   bgzf_close(t->fp); ti_index_destroy(t->idx);
	   free(t);
	}
}

MODULE = Tabix PACKAGE = Tabix

tabix_t*
tabix_open(fn)
	char *fn
  PROTOTYPE: $
  CODE:
	RETVAL = _tabix_open(fn);
  OUTPUT:
	RETVAL

void
tabix_close(t)
	tabix_t *t
  PROTOTYPE: $
  CODE:
	_tabix_close(t);

void
tabix_DESTROY(t)
	tabix_t *t
  PROTOTYPE: $
  CODE:
	_tabix_close(t);

ti_iter_t
tabix_query(t, seq=0, beg=0, end=0x7fffffff)
	tabix_t *t
	const char *seq
	int beg
	int end
  PREINIT:
	int tid;
  CODE:
	if (seq) {
	   if (beg < 0) beg = 0;
	   if (end < beg) end = beg + 1;
	   if ((tid = ti_get_tid(t->idx, seq)) < 0)
	   	  return XSRETURN_EMPTY;
	   RETVAL = ti_query(t->fp, t->idx, tid, beg, end);
	} else RETVAL = ti_first(t->fp);
  OUTPUT:
	RETVAL

SV*
tabix_read(iter)
	ti_iter_t iter
  PROTOTYPE: $
  PREINIT:
	const char *s;
	int len;
  CODE:
	s = ti_iter_read(iter, &len);
	if (s == 0)
	   return XSRETURN_EMPTY;
	RETVAL = newSVpv(s, len);
  OUTPUT:
	RETVAL

void
tabix_getnames(t)
	tabix_t *t
  PREINIT:
	const char **names;
	int i, n;
  PPCODE:
	names = ti_seqname(t->idx, &n);
	for (i = 0; i < n; ++i)
		XPUSHs(sv_2mortal(newSVpv(names[i], 0)));
	free(names);
