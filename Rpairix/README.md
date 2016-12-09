# Rpairix

R pairix is an R binder for pairix, which allows querying on pairix-indexed bgzipped text files.


## Installation
```
make
R CMD SHLIB rpairix.c index.o bgzf.o --preclean
```

## Usage
The `px_query` function can be used on R. 
```
source("rpairix.r")
px_query(filename,querystr,max_mem=8000)
```
The filename is sometextfile.gz and an index file sometextfile.gz.px2 must exist.
The query string is in the same format as the format for pairix. (e.g. '1:1-10000000|20:50000000-60000000')
The max_mem is the maximum total length of the result strings (sum of string lengths). 

```
source("rpairix.r")

## example run
filename = "../samples/merged_nodup.tab.chrblock_sorted.txt.gz"
querystr = "10:1-1000000|20"
res = px_query(filename,querystr)
print(res)
```
The result would look as below:
```
  V1 V2     V3   V4 V5 V6       V7     V8
1  0 10 624779 1361  0 20 40941397  97868
2 16 10 948577 2120 16 20 59816485 148396
```

## Return value
Return value is a data frame, each row corresponding to the line in the input file within the query range.


