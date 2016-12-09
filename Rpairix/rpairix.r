dyn.load("rpairix.so")

px_query<-function(filename, querystr, max_mem=8000){

  # first-round, get the max length and the number of lines of the result.
  out =.C("get_size", filename, querystr, as.integer(0), as.integer(0))
  str_len = out[[4]][1]
  n=out[[3]][1]
  total_size = str_len * n
  if(total_size > max_mem) {
     log = paste("not enough memory: Total length of the result to be stored is",total_size,sep=" ")
     message(log)
     return(NULL)
  }

  # second-round, actually get the lines from the file
  result_str = rep(paste(rep("a",str_len),collapse=''),n)
  out2 =.C("get_lines", filename , querystr, result_str)

  ## tabularize
  res.table = as.data.frame(do.call("rbind",strsplit(out2[[3]],'\t')),stringsAsFactors=F)
  return (res.table)
}

px_keylist<-function(filename){
   # first-round, get the max length and the number of items in the key list.
   out = .C("get_keylist_size", filename, as.integer(0), as.integer(0))
   max_key_len = out[[3]][1]
   n = out[[2]][1]
  
   # second-round, get the key list.
   result_str = rep(paste(rep("a",max_key_len),collapse=''),n)
   out = .C("get_keylist", filename, result_str)

   return(out[[2]])
}

px_seq1list<-function(filename){
  seqpairs = px_keylist(filename)
  seq1_list = unique(sapply(seqpairs,function(xx)strsplit(xx,'|',fixed=T)[[1]][1]))
  return(sort(seq1_list))
}

px_seq2list<-function(filename){
  seqpairs = px_keylist(filename)
  seq2_list = unique(sapply(seqpairs,function(xx)strsplit(xx,'|',fixed=T)[[1]][2]))
  return(sort(seq2_list))
}

px_seqlist<-function(filename){
  seqpairs = px_keylist(filename)
  seq1_list = unique(sapply(seqpairs,function(xx)strsplit(xx,'|',fixed=T)[[1]][1]))
  seq2_list = unique(sapply(seqpairs,function(xx)strsplit(xx,'|',fixed=T)[[1]][2]))
  seq_list = unique(c(seq1_list, seq2_list))
  return(sort(seq_list))
}

