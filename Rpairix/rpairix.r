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

