# Rpairix

R pairix is an R binder for pairix, which allows querying on pairix-indexed bgzipped text files.


## Installation
```
make
R CMD SHLIB rpairix.c index.o bgzf.o --preclean
```

## Usage
The `px_query` function can be used on R. 
The filename is sometextfile.gz and an index file sometextfile.gz.px2 must exist.
The query string is in the same format as the format for pairix. (e.g. '1:1-10000000|20:50000000-60000000')
The max_mem is the maximum total length of the result strings (sum of string lengths). 
```
R
>source("rpairix.r")
>px_query(filename, querystr, max_mem=8000)
```

## Return value
Return value is a data frame, each row corresponding to the line in the input file within the query range.


