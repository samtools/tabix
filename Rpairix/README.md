# Rpairix

R pairix is an R binder for pairix, which allows querying on pairix-indexed bgzipped text files.


## Installation
```
make
R CMD SHLIB rpairix.c index.o bgzf.o --preclean
```

## Usage
Functions `px_query` and `px_keylist` can be used on R. 
```
source("rpairix.r")
px_query(filename,querystr,max_mem=8000)
```
* The filename is sometextfile.gz and an index file sometextfile.gz.px2 must exist.
* The query string is in the same format as the format for pairix. (e.g. '1:1-10000000|20:50000000-60000000')
* The max_mem is the maximum total length of the result strings (sum of string lengths). 
* The return value is a data frame, each row corresponding to the line in the input file within the query range.

```
source("rpairix.r")
px_keylist(filename)
```
* The filename is sometextfile.gz and an index file sometextfile.gz.px2 must exist.
* The return value is a vector of keys (chromosome pairs).

```
source("rpairix.r")
px_seqlist(filename)
```
* The filename is sometextfile.gz and an index file sometextfile.gz.px2 must exist.
* The return value is a vector of chromosomes.

```
source("rpairix.r")
px_seq1list(filename)
```
* The filename is sometextfile.gz and an index file sometextfile.gz.px2 must exist.
* The return value is a vector of first chromosomes.

```
source("rpairix.r")
px_seq2list(filename)
```
* The filename is sometextfile.gz and an index file sometextfile.gz.px2 must exist.
* The return value is a vector of second chromosomes.


## Example run
```
> source("rpairix.r")
> filename = "../samples/merged_nodup.tab.chrblock_sorted.txt.gz"
> querystr = "10:1-1000000|20"
> res = px_query(filename,querystr)
> print(res)
  V1 V2     V3   V4 V5 V6       V7     V8
1  0 10 624779 1361  0 20 40941397  97868
2 16 10 948577 2120 16 20 59816485 148396
>
> keys = px_keylist("../samples/merged_nodup.tab.chrblock_sorted.txt.gz")
> length(keys)
[1] 1239
> keys[1:10]
[1] "1|1"  "1|10" "1|11" "1|12" "1|13" "1|14" "1|15" "1|16" "1|17" "1|18"
>
>chrs = px_seqlist("../samples/merged_nodup.tab.chrblock_sorted.txt.gz")
>chrs
 [1] "1"          "10"         "11"         "12"         "13"        
 [6] "14"         "15"         "16"         "17"         "18"        
[11] "19"         "2"          "20"         "21"         "22"        
[16] "3"          "4"          "5"          "6"          "7"         
[21] "8"          "9"          "GL000191.1" "GL000192.1" "GL000193.1"
[26] "GL000194.1" "GL000195.1" "GL000196.1" "GL000197.1" "GL000198.1"
[31] "GL000199.1" "GL000200.1" "GL000201.1" "GL000202.1" "GL000203.1"
[36] "GL000204.1" "GL000205.1" "GL000206.1" "GL000208.1" "GL000209.1"
[41] "GL000210.1" "GL000211.1" "GL000212.1" "GL000213.1" "GL000214.1"
[46] "GL000215.1" "GL000216.1" "GL000217.1" "GL000218.1" "GL000219.1"
[51] "GL000220.1" "GL000221.1" "GL000222.1" "GL000223.1" "GL000224.1"
[56] "GL000225.1" "GL000226.1" "GL000227.1" "GL000228.1" "GL000229.1"
[61] "GL000230.1" "GL000231.1" "GL000232.1" "GL000233.1" "GL000234.1"
[66] "GL000235.1" "GL000236.1" "GL000237.1" "GL000238.1" "GL000239.1"
[71] "GL000240.1" "GL000241.1" "GL000242.1" "GL000243.1" "GL000244.1"
[76] "GL000245.1" "GL000246.1" "GL000247.1" "GL000248.1" "GL000249.1"
[81] "MT"         "NC_007605"  "X"          "Y"     
```



