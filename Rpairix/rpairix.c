#include "pairix.h"

// load
tabix_t *load(char* fn){

  tabix_t *tb=NULL; 

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

   int len=-1;
   const char *s;

   tabix_t *tb = load(*pfn);
   const ti_conf_t *idxconf = ti_get_conf(tb->idx);

   ti_iter_t iter = ti_querys_2d(tb, *pquerystr);

   *pn=0;
   *pmax_len=0;
   while ((s = ti_read(tb, iter, &len)) != 0) {
     // if ((int)(*s) != idxconf->meta_char) break;    // I don't fully understand this line. Works without the line.
     if(len>*pmax_len) *pmax_len = len;
     (*pn)++;
   }

   ti_close(tb);
}


//load + return the result
void get_lines(char** pfn, char** pquerystr, char** presultstr){

   int len=-1;
   const char *s;
   int k=0;

   tabix_t *tb = load(*pfn);
   const ti_conf_t *idxconf = ti_get_conf(tb->idx);
   ti_iter_t iter = ti_querys_2d(tb, *pquerystr);

   while ((s = ti_read(tb, iter, &len)) != 0) {
     // if ((int)(*s) != idxconf->meta_char) break;    // I don't fully understand this line. Works without the line.
     strcpy(presultstr[k++],s);
   }

   ti_close(tb);
}


//get number of seq(chr)pairs
void get_keylist_size(char** pfn, int *pn, int* pmax_key_len){
  int len;
  int i;
  tabix_t *tb = load(*pfn);
  char** ss = ti_seqname(tb->idx, pn);
  *pmax_key_len=0;
  for(i=0;i<*pn;i++){
    len=strlen(ss[i]);
    if(len>*pmax_key_len) *pmax_key_len=len;
  }
  free(ss);
  ti_close(tb);
}

//get the list of seq(chr)pair
void get_keylist(char** pfn, char** pkeylist){
  int n,i;
  tabix_t *tb = load(*pfn);
  char **ss = ti_seqname(tb->idx, &n);
  for(i=0;i<n;i++){
    strcpy(pkeylist[i],ss[i]);
  }
  free(ss);
  ti_close(tb);
}


