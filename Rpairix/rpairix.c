#include "pairix.h"

// load
tabix_t *load(char* fn){

  tabix_t *tb; 

  // index file name
  char fnidx[strlen(fn)+5];
  strcpy(fnidx,fn);
  strcpy(fnidx+strlen(fn),".px2");

  // open file
  tb = ti_open(fn, fnidx);  // add: check file exists?
  tb->idx = ti_index_load(fn);
  
  return(tb);
}


//load + get size of the query result
void get_size(char** pfn, char** pquerystr, int* pn, int* pmax_len){

   int len;
   const char *s;

   tabix_t *tb = load(*pfn);
   const ti_conf_t *idxconf = ti_get_conf(tb->idx);
   ti_iter_t iter = ti_querys_2d(tb, *pquerystr);

   while ((s = ti_read(tb, iter, &len)) != 0) {
     if ((int)(*s) != idxconf->meta_char) break;   
     if(len>*pmax_len) *pmax_len = len;
     (*pn)++;
   }

}


