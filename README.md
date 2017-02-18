# pairix
* Pairix is a tool for random accessing a compressed text file. Specifically, it does indexing and querying on a bgzipped text file that contains a pair of genomic coordinates per line (a pairs file).
* As an example, you have a text file with millions or bilions of lines like below, and you want to extract lines in (chr10,chrX) pair. An awk command would read the file from the beginning till you find the pair. However, if your file is sorted by chromosome pair and indexed, you can extract the lines almost instantly. That's what pairix does.:
 
  ex1)
  ```
  chr1  10000  20000 chr2  30000  50000
  chr1  30000  40000 chr3  10000  70000
  ```
  ex2)
  ```
  chr1  10000  +  chr2  30000  -
  chr1  30000  -  chr3  50000  -
  ```
  
## Table of contents
* [Availability](#availability)
* [Input file format](#input-file-format)
* [Pairix](#pairix)
    * [Installation](#installation-for-pairix)
    * [Usage](#usage-for-pairix)
    * [Examples](#usage-examples-for-pairix)
* [Pypairix](#pypairix)
    * [Installation](#installation-for-pypairix)
    * [Examples](#usage-examples-for-pypairix)
* [Rpairix](#rpairix)
* [Utils](#utils)
    * [process_merged_nodup.sh](#process_merged_nodup.sh)
    * [process_old_merged_nodup.sh](#process_old_merged_nodup.sh)
* [Pairs_merger](#pairs_merger)
    * [Installation](#installation-for-pairs_merger)
    * [Usage](#usage-for-pairs_merger)
    * [Examples](#usage-examples-for-pairs_merger)
* [Streamer_1d](#streamer_1d)
    * [Installation](#installation-for-streamer-1d)
    * [Usage](#usage-for-streamer-1d)
    * [Examples](#usage-examples-for-streamer-1d)
* [Difference between pairix and tabix](#difference-between-pairix-and-tabix)
* [Note](#note)

## Availability
* Pairix is available either as a stand-alone command-line program, a python library (pypairix), and an R package (Rpairix https://github.com/4dn-dcic/Rpairix)
* Pairs_merger is available to merge two or more indexed pairs files.
* Bgzip is provided as part of the repo, which is identical to the original program.

## Input file format
* The text file must be first sorted by two chromosome columns and then by the first position column. The file must be compressed using bgzip. The file can be either tab-delimited or space-delimited.
* The index file has an extension .px2.

## Pairix
### Installation for pairix
The same command installs bgzip and pairs_merger as well.
```
git clone https://github.com/4dn-dcic/pairix
cd pairix
make
# add the bin path to PATH
```

### Usage for pairix
```
# compression
bgzip textfile

# indexing
pairix -s<chr1_column> [-d<chr2_column>] -b<pos1_start_column> -e<pos1_end_column> [-u<pos2_start_column> -v<pos2_end_column>] [-T] textfile.gz    # u, v is required for full 2d query.
# column indices are 1-based.
# use -T option for a space-delimited file.


# querying
pairix textfile.gz region1 [region2 [...]]  ## region is in the following format.
pairix textfile.gz '<chr1>:<start1>-<end1>[|<chr2>:<start2>-<end2>]'    # make sure to quote, so '|' is not interpreted as a pipe.
```

### Usage examples for pairix

#### Preparing a 4dn-style pairs file. This is a double-chromosome-block sorted test file.
(column 2 and 4 are chromosomes (chr1 and chr2), column 3 is position of the first coordinate (pos1)).
```
sort -k2,2 -k4,4 -k3,3 -k5,5 samples/4dn.bsorted.chr21_22_only.pairs |bgzip -c > samples/4dn.bsorted.chr21_22_only.pairs.gz
pairix -f -s2 -b3 -e3 -d4 -u5 samples/4dn.bsorted.chr21_22_only.pairs.gz
```

#### Preparing a double-chromosome-block sorted merged_nodups.txt file (Juicer style pairs file)
(column 2 and 6 are chromosomes (chr1 and chr2), column 3 is position of the first coordinate (pos1)).
```
# the following has already be done and the final merged_nodup.tab.chrblock_sorted.txt.gz is already in the samples folder.
cut -d' ' -f1-8 /n/data1/hms/dbmi/park/sl325/juicer/SRR1658832/aligned.20160803/merged_nodups.txt | sort -t' ' -k2,2 -k6,6 -k3,3g - > merged_nodup.chrblock_sorted.txt
sed 's/ /\t/g' merged_nodup.chrblock_sorted.txt > samples/merged_nodup.tab.chrblock_sorted.txt   ## note that sed may not work for every implementation.
head samples/merged_nodup.tab.chrblock_sorted.txt

0	1	49819	93	0	1	16858344	44945
0	1	108364	242	16	1	255090	508
16	1	108871	246	16	1	222643221	508360
0	1	110012	248	16	1	116929	264
0	1	114876	256	0	1	156133	357
0	1	122900	271	0	1	224115759	511974
0	1	128053	280	0	1	129622	284
16	1	128155	280	16	1	251467	498
0	1	139312	305	0	1	141553	315
0	1	142396	316	0	1	222981299	509189
```
bgzipping
```
bgzip samples/merged_nodup.tab.chrblock_sorted.txt
```

#### 2D indexing & query on the above file
2D indexing with pairix on chromosome pair (-s2 -d6) and the position of the first chromosome (b3). For full 2D query, also add -u7 and -v7, the start and end positions of the second coordinate. They are not used for indexing per se, but the column index is stored as part of the index, which allows full 2D query through individual comparisons.
```
pairix -f -s2 -d6 -b3 -e3 -u7 samples/merged_nodup.tab.chrblock_sorted.txt.gz
```
semi 2D query (chr10:1-1000000 x chr20)
```
pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '10:1-1000000|20'
0	10	624779	1361	0	20	40941397	97868
16	10	948577	2120	16	20	59816485	148396
```
full 2D query (chr10:1-1000000 x chr20:50000000-60000000)
```
pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '10:1-1000000|20:50000000-60000000'
16	10	948577	2120	16	20	59816485	148396
```
full 2D multi-query (chr1:1-10000000 x chr20:50000000-60000000 AND 3:5000000-9000000 x X:70000000-90000000)
```
pairix samples/merged_nodup.tab.chrblock_sorted.txt.gz '1:1-10000000|20:50000000-60000000' '3:5000000-9000000|X:70000000-90000000'
16	1	4717358	10139	16	20	55598650	138321
0	1	5649238	12370	16	20	59660150	148059
16	1	6651242	15069	0	20	50444303	124692
0	1	6930805	15906	16	20	50655496	125483
0	1	8555535	20339	0	20	55253919	137318
16	3	5025392	11911	16	X	86766531	207787
0	3	5298790	12678	0	X	84731179	203102
0	3	7272964	17297	0	X	88560374	211726
16	3	8402388	19935	16	X	77717595	187377
```

#### 2D indexing & query on a space-delimited file
The -T option is for space-delimited files.
```
pairix -T -f -s2 -d6 -b3 -e3 -u7 samples/merged_nodups.space.chrblock_sorted.subsample1.txt.gz
```
Query commands are the same as above (no need to use -T option for query).


#### 1D indexing on a regular vcf file, bgzipped.
1D indexing
```
pairix -s1 -b2 -e2 -f samples/SRR1171591.variants.snp.vqsr.p.vcf.gz
```
1D query
```
pairix samples/SRR1171591.variants.snp.vqsr.p.vcf.gz chr10:1-4000000
chr10	3463966	.	C	T	51.74	PASS	AC=2;AF=1.00;AN=2;DB;DP=2;Dels=0.00;FS=0.000;HaplotypeScore=0.0000;MLEAC=2;MLEAF=1.00;MQ=50.00;MQ0=0;POSITIVE_TRAIN_SITE;QD=25.87;VQSLOD=7.88;culprit=FS	GT:AD:DP:GQ:PL	1/1:0,2:2:6:79,6,0
chr10	3978708	rs29320259	T	C	1916.77	PASS	AC=2;AF=1.00;AN=2;BaseQRankSum=1.016;DB;DP=67;Dels=0.00;FS=0.000;HaplotypeScore=629.1968;MLEAC=2;MLEAF=1.00;MQ=50.00;MQ0=0;MQRankSum=0.773;POSITIVE_TRAIN_SITE;QD=28.61;ReadPosRankSum=0.500;VQSLOD=3.29;culprit=FS	GT:AD:DP:GQ:PL	1/1:3,64:67:70:1945,70,0
chr10	3978709	.	G	A	1901.77	PASS	AC=2;AF=1.00;AN=2;BaseQRankSum=0.677;DB;DP=66;Dels=0.00;FS=0.000;HaplotypeScore=579.9049;MLEAC=2;MLEAF=1.00;MQ=50.00;MQ0=0;MQRankSum=0.308;POSITIVE_TRAIN_SITE;QD=28.81;ReadPosRankSum=0.585;VQSLOD=3.24;culprit=FS	GT:AD:DP:GQ:PL	1/1:3,63:66:73:1930,73,0
```


## Pypairix
### Installation for pypairix

```
# to install the python module pypairix,
pip install pypairix
# you may need to install python-dev for some ubuntu releases.

# or
cd pairix
python setup.py install

# testing the python module
python test/test.py
```


### Usage examples for pypairix
```
# to import and use python module pypairix, add the following in your python script.
import pypairix

# 2D query usage example 1 with `query2D(chrom, start, end, chrom2, start2, end2)`
tb=pypairix.open("textfile.gz")
it = tb.query2D(chrom, start, end, chrom2, start2, end2)
for x in it:
   print(x)

# 2D query usage example 1 with *autoflip* with `query2D(chrom, start, end, chrom2, start2, end2, 1)`
# Autoflip: if the queried chromosome pair does not exist in the pairs file, query the flipped pair.
tb=pypairix.open("textfile.gz")
it = tb.query2D(chrom2, start2, end2, chrom1, start1, end1, 1)
tb_result = [[x[1], x[2], x[2], x[5], x[6], x[6]] for x in it]
for x in it:
   print(x)

# 2D query usage example 2 with `querys2D(querystr)`
tb=pypairix.open("textfile.gz")
querystr='{}:{}-{}|{}:{}-{}'.format(chrom, start, end, chrom2, start2, end2)
it = tb.querys2D(querystr)
for x in it:
   print(x)
   
# 2D query usage example 3, with *autoflip* with `querys2D(querystr, 1)`
# Autoflip: if the queried chromosome pair does not exist in the pairs file, query the flipped pair.
tb=pypairix.open("textfile.gz")
querystr='{}:{}-{}|{}:{}-{}'.format(chrom2, start2, end2, chrom, start, end)
it = tb.querys2D(querystr, 1)
for x in it:
   print(x)
   
# 1D query usage example 1
tb=pypairix.open("textfile.gz")
it = tb.query(chrom, start, end)
for x in it:
   print(x)
   
# 1D query usage example 2
tb=pypairix.open("textfile.gz")
querystr='{}:{}-{}'.format(chrom, start, end)
it = tb.querys2D(querystr)
for x in it:
   print(x)
   
# get the list of (chr-pair) blocks
tb=pypairix.open("textfile.gz")
chrplist = tb.get_blocknames()
print str(chrplist)

```

## Rpairix
* Rpairix is an R package for reading pairix-indexed pairs files. It has its own repo: https://github.com/4dn-dcic/Rpairix

## Utils
### process_merged_nodup.sh 
* This script sorts, bgzips and indexes a newer version of merged_nodups.txt file with strand1 as the first column.
```
Usage: process_merged_nodup.sh <merged_nodups.txt>
```

### process_old_merged_nodup.sh 
* This script sorts, bgzips and indexes an old version of merged_nodups.txt file with readID as the first column.
```
Usage: process_old_merged_nodup.sh <merged_nodups.txt>
```

## Pairs_merger
Pairs_merger is a tool that merges indexed pairs files that are already sorted and creates a sorted output pairs file. Pairs_merger uses a k-way merge sort algorithm starting with k file streams. Specifically, it loops over a merged iterator composed of a dynamically sorted array of k interators. It does not require additional memory nor produces temporary files.

### Installation for pairs_merger
See [Installation for pairix](#installation-for-pairix)

### Usage for pairs_merger
```
pairs_merger <in1.gz> <in2.gz> <in3.gz> ... > <out.txt>
# Each of the input files must have a .px2 index file.
bgzip out.txt

## or pipe to bgzip
pairs_merger <in1.gz> <in2.gz> <in3.gz> ... | bgzip -c > <out.gz>

# To index the output file as well
# use the appropriate options according to the output file format.
pairix [options] out.gz
```

### Usage Examples for pairs_merger
```
bin/pairs_merger samples/merged_nodups.space.chrblock_sorted.subsample2.txt.gz samples/merged_nodups.space.chrblock_sorted.subsample3.txt.gz | bin/bgzip -c > out.gz
bin/pairix -f -s2 -d6 -b3 -e3 -u7 -T out.gz
```

 
## Streamer_1d
Streamer_1d is a tool that converts a 2d-sorted pairs file to a 1d-sorted stream (sorted by chr1-chr2-pos1-pos2  ->  sorted by chr1-pos1). This tool uses a k-way merge sort on k file pointers on the same input file, operates linearly without producing any temporary files. Currently, the speed is actually slower than unix sort (not recommended). 

### Installation for streamer_1d
See [Installation for pairix](#installation-for-pairix)

### Usage for streamer_1d
```
streamer_1d <in.2d.gz> > out.1d.pairs
streamer_1d <in.2d.gz> | bgzip -c > out.1d.pairs.gz
```

### Usage Examples for streamer_1d


### FAQ for streamer_1d
#### The tool creates many file pointers for the input file, which is equivalent to opening many files simultaneously. Your OS may have a limit on the number of files that can be open at a time. For example, for Mac El Captain and Sierra, it is by default set to 256. This is usually enough, but in case the number of chromosomes in your pairs file happen to be larger than or close to this limit, the tool may produce an error message saying file limit is exceeded. You can increase this limit outside the program. For example, for Mac El Captain and Sierra, the following command raises the limit to 2000.
```
# view the limits
uimit -a

# raise the limit to 2000
ulimit -n 2000
```

## Difference between pairix and tabix
* If you're thinking "Sounds familiar.. How is it different from tabix?"
  * Pairix was created by modifying tabix, and the major difference is that pairix create an index based on a pair of chromosome columns instead of a single colume.
  * Pairix has added functionality of 2D query.
  * Pairix comes with a pairs_merger util for fast merging of sorted pairs files, that makes use of the index.
  * Pairix can handle space-delimited files as well as tab-delimited files.
  * Tabix and pairix are not cross-compatible, although pairix can optionally index based on a single colume. The index structure had to change to accomodate the double-colume requirement. If you want to create a single-colume index, it is recommended to use Tabix, to avoid potential confusion.
 
 
## Note
* Currently 2D indexing supports only 2D query and 1D indexing supports only 1D query. Ideally, it will be extended to support 1D query for 2D indexed files. (future plan)
* The index produced by this modified pairix is not compatible with the original tabix index. They are based on different structures.
* Note that if the chromosome pair block are ordered in a way that the first coordinate is always smaller than the second ('upper-triangle'), a lower-triangle query will return an empty result. For example, if there is a block with chr1='6' and chr2='X', but not with chr1='X' and chr2='6', then the query for X|6 will not return any result. The search is not symmetric.



