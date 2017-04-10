# pairix
* Pairix is a tool for indexing and querying on a block-compressed text file containing a pair of genomic coordinates.
* Pairix is a stand-alone C program that was written on top of tabix (https://github.com/samtools/tabix) as a tool for the 4DN-standard pairs file format describing Hi-C data: [pairs_format_specification.md](pairs_format_specification.md)
* However, Pairix can be used as a generic tool for indexing and querying any bgzipped text file containing genomic coordinates, for either 2D- or 1D- indexing and querying.
* For example, given a text file like below, you want to extract specific lines. An awk command, for example, would read the file from the beginning to the end. Pairix creates an index and uses it to accesses the file from a relevant position by taking advantage of the bgzf compression, allowing for a fast query for large files.

  **Pairs format**
  ```
  ## pairs format v1.0
  #sorted: chr1-chr2-pos1-pos2
  #shape: upper triangle
  #chromosomes: chr1 chr2 chr3 chr4 chr5 chr6 chr7 chr8 chr9 chr10 chr11 chr12 chr13 chr14 chr15 chr16 chr17 chr18 chr19 chr20 chr21 chr22 chrX chrY chrM
  #genome_assembly: hg38
  #columns: readID chr1 pos1 chr2 pos2 strand1 strand2
  EAS139:136:FC706VJ:2:2104:23462:197393 chr1 10000 chr1 20000 + +
  EAS139:136:FC706VJ:2:8762:23765:128766 chr1 50000 chr1 70000 + +
  EAS139:136:FC706VJ:2:2342:15343:9863 chr1 60000 chr2 10000 + +
  EAS139:136:FC706VJ:2:1286:25:275154 chr1 30000 chr3 40000 + -
  ```

  **Some custom text file**
  ```
  chr1  10000  20000 chr2  30000  50000  3.5
  chr1  30000  40000 chr3  10000  70000  4.6
  ```

* Bgzip can be found either in this repo or https://github.com/samtools/tabix (original).


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
    * [bam2pairs](#bam2pairs)
    * [process_merged_nodup.sh](#process_merged_nodupsh)
    * [process_old_merged_nodup.sh](#process_old_merged_nodupsh)
    * [merged_nodup2pairs.pl](#merged_nodup2pairspl)
    * [old_merged_nodup2pairs.pl](#old_merged_nodup2pairspl)
    * [Pairs_merger](#pairs_merger)
        * [Usage](#usage-for-pairs_merger)
        * [Examples](#usage-examples-for-pairs_merger)
    * [Streamer_1d](#streamer_1d)
        * [Usage](#usage-for-streamer-1d)
        * [Examples](#usage-examples-for-streamer-1d)
* [Note](#note)
* [Version history](#version-history)

<br>

## Availability
* Pairix is available either as a stand-alone command-line program, a python library (pypairix), and an R package (Rpairix https://github.com/4dn-dcic/Rpairix)
* Various utils including `bam2pairs`, `merged_nodups2pairs.pl`, `pairs_merger` etc. are available within this repo.
* The `bgzip` program that is provided as part of the repo is identical to the original program in https://github.com/samtools/tabix.

<br>

## Input file format
* For 2D indexing, the text file must be first sorted by two chromosome columns and then by the first position column. For 1D-indexing, the file must be sorted by a chromosome column and then by a position column.
* The file must be compressed using bgzip. The file is either tab-delimited or space-delimited.
* The index file has an extension `.px2`.

<br>

## Pairix
### Installation for pairix
```
git clone https://github.com/4dn-dcic/pairix
cd pairix
make
# Add the bin path to PATH for pairix, bgzip, pairs_merger and stream_1d
# In order to use utils, add util path to PATH
# In order to use bam2pairs, add util/bam2pairs to PATH
# eg: PATH=~/git/pairix/bin/:~/git/pairix/util:~/git/pairix/util/bam2pairs:$PATH
```

<br>

### Usage for pairix
#### compression
```
bgzip textfile
```

#### indexing
```
pairix textfile.gz  # for recognized file extension
pairix -p <preset> textfile.gz
pairix -s<chr1_column> [-d<chr2_column>] -b<pos1_start_column> -e<pos1_end_column> [-u<pos2_start_column> -v<pos2_end_column>] [-T] textfile.gz    # u, v is required for full 2d query.
```
* column indices are 1-based.
* use `-T` option for a space-delimited file.
* use `-f` option to overwrite an existing index file.
* presets can be used for indexing : `pairs`, `merged_nodups`, `old_merged_nodups` for 2D indexing, `gff`, `vcf`, `bed`, `sam` for 1D indexing. Default is `pairs`.
* For the recognized file extensions, the `-p` option can be dropped: `.pairs.gz`, `.vcf.gz`, `gff.gz`, `bed.gz`, `sam.gz`
* Custom column specification (-s, -d, -b, -e, -u, -v) overrides file extension recognition. The custom specification must always have at least chr1_column (-s).

#### querying
```
pairix textfile.gz region1 [region2 [...]]  ## region is in the following format.

# for 1D indexed file
pairix textfile.gz '<chr>:<start>-<end>' '<chr>:<start>-<end>' ...

# for 2D indexed file
pairix textfile.gz '<chr1>:<start1>-<end1>|<chr2>:<start2>-<end2>' ...    # make sure to quote, so '|' is not interpreted as a pipe.
pairix textfile.gz '*|<chr2>:<start2>-<end2>'  # wild card is accepted for 1D query on 2D indexed file
pairix textfile.gz '<chr1>:<start1>-<end1>|*' # wild card is accepted for 1D query on 2D indexed file

# using a file listing query regions
pairix -L textfile.gz regionfile1 [regionfile2 [...]] # region file contains one region string per line
```

<br>

### Usage examples for pairix

#### Preparing a 4dn-style pairs file. This is a double-chromosome-block sorted test file.
(column 2 and 4 are chromosomes (chr1 and chr2), column 3 is position of the first coordinate (pos1)).
```
# sorting & bgzipping
sort -k2,2 -k4,4 -k3,3n -k5,5n samples/4dn.bsorted.chr21_22_only.pairs |bgzip -c > samples/4dn.bsorted.chr21_22_only.pairs.gz

# indexing
pairix -f samples/4dn.bsorted.chr21_22_only.pairs.gz
# The above command is equivalent to: pairix -f -s2 -b3 -e3 -d4 -u5 samples/4dn.bsorted.chr21_22_only.pairs.gz
# The above command is also equivalent to: pairix -f -p pairs samples/4dn.bsorted.chr21_22_only.pairs.gz
# Pairs extension .pairs.gz is automatically recognized.
```

#### Preparing a double-chromosome-block sorted `merged_nodups.txt` file (Juicer style pairs file)
(column 2 and 6 are chromosomes (chr1 and chr2), column 3 is position of the first coordinate (pos1)).
```
# sorting & bgzipping
sort -t' ' -k2,2 -k6,6 -k3,3n -k7,7n merged_nodups.txt |bgzip -c > samples/merged_nodups.space.chrblock_sorted.subsample3.txt

#indexing
pairix -f -p merged_nodups samples/merged_nodups.space.chrblock_sorted.subsample3.txt.gz
# The above command is equivalent to : pairix -f -s2 -d6 -b3 -e3 -u7 -T samples/merged_nodups.space.chrblock_sorted.subsample3.txt.gz
```

#### querying
semi 2D query with two chromosomes
```
pairix samples/test_4dn.pairs.gz 'chr21|chr22'
SRR1658581.33025893	chr21	9712946	chr22	21680462	-	+
SRR1658581.9428560	chr21	10774937	chr22	37645396	-	+
SRR1658581.8816993	chr21	11171003	chr22	33169971	+	+
SRR1658581.10673815	chr21	16085548	chr22	35451128	+	-
SRR1658581.2504661	chr21	25672432	chr22	21407301	-	-
SRR1658581.40524826	chr21	28876237	chr22	42449178	+	+
SRR1658581.8969171	chr21	33439464	chr22	43912252	-	-
SRR1658581.6842680	chr21	35467614	chr22	33478115	+	+
SRR1658581.15363628	chr21	37956917	chr22	21286436	-	-
SRR1658581.3572823	chr21	40651454	chr22	41358228	-	-
SRR1658581.50137399	chr21	42446807	chr22	49868647	-	+
SRR1658581.11358652	chr21	43768599	chr22	40759935	-	-
SRR1658581.4127782	chr21	45142744	chr22	36929446	+	+
SRR1658581.38401094	chr21	46989766	chr22	45627553	-	+
SRR1658581.34261420	chr21	48113817	chr22	51138644	+	-
```

semi 2D query with a chromosome and a range
```
pairix samples/test_4dn.pairs.gz 'chr21:10000000-20000000|chr22'
SRR1658581.9428560	chr21	10774937	chr22	37645396	-	+
SRR1658581.8816993	chr21	11171003	chr22	33169971	+	+
SRR1658581.10673815	chr21	16085548	chr22	35451128	+	-
```

full 2D query with two ranges
```
pairix samples/test_4dn.pairs.gz 'chr21:10000000-20000000|chr22:30000000-35000000'
SRR1658581.8816993	chr21	11171003	chr22	33169971	+	+
```

full 2D multi-query
```
pairix samples/test_4dn.pairs.gz 'chr21:10000000-20000000|chr22:30000000-35000000' 'chrX:100000000-110000000|chrX:150000000-170000000'
SRR1658581.8816993	chr21	11171003	chr22	33169971	+	+
SRR1658581.39700722	chrX	100748075	chrX	154920234	+	+
SRR1658581.36337371	chrX	104718152	chrX	151646254	+	-
SRR1658581.49591338	chrX	104951264	chrX	154363440	+	+
SRR1658581.46205223	chrX	105732382	chrX	155162659	+	-
SRR1658581.32048997	chrX	107326643	chrX	151899433	-	+
```

Wild-card 2D query
```
pairix samples/test_4dn.pairs.gz 'chr21:9000000-9700000|*'
SRR1658581.18102003	chr21	9582382	chr21	9733996	+	+
SRR1658581.10121427	chr21	9665774	chr4	49203518	+	-
SRR1658581.1019708	chr21	9496682	chr4_gl000193_random	48672	+	+
SRR1658581.44516250	chr21	9662891	chr6	7280832	-	+
SRR1658581.15515341	chr21	9549471	chr9	68384076	+	+
SRR1658581.51399686	chr21	9687495	chrUn_gl000221	87886	+	+
SRR1658581.25532108	chr21	9519859	chrUn_gl000226	6821	+	-
SRR1658581.22081000	chr21	9659013	chrUn_gl000232	19626	-	-
SRR1658581.34308344	chr21	9532618	chrX	61793091	-	+
```
```
pairix samples/test_4dn.pairs.gz '*|chr21:9000000-9700000'
SRR1658581.21313395	chr1	25612365	chr21	9679403	+	-
SRR1658581.46040617	chr1	143255816	chr21	9663103	+	+
SRR1658581.54790470	chr14	101961336	chr21	9481250	+	+
SRR1658581.38248307	chr18	18518988	chr21	9452846	-	+
SRR1658581.9143926	chr2	90452598	chr21	9486716	+	-
```

Query using a region file
```
cat samples/test.regions
chr1:1-50000|*
*|chr1:1-50000
chr2:1-20000|*
*|chr2:1-20000

cat samples/test.regions2
chrX:100000000-110000000|chrY
chr19:1-100000|chr19

bin/pairix -L samples/test_4dn.pairs.gz samples/test.regions samples/test.regions2
SRR1658581.49364897	chr1	36379	chr20	62713042	+	+
SRR1658581.31672330	chr1	12627	chr9	23963238	+	-
SRR1658581.22713561	chr1	14377	chrX	107423076	-	+
SRR1658581.31992022	chrX	108223782	chrY	5017118	-	-
```


#### 1D indexing on a regular vcf file, bgzipped.
1D indexing
```
pairix -f samples/SRR1171591.variants.snp.vqsr.p.vcf.gz
# The above command is equivalent to : pairix -f -s1 -b2 -e2 samples/SRR1171591.variants.snp.vqsr.p.vcf.gz
# The above command is also equivalent to : pairix -f -p vcf samples/SRR1171591.variants.snp.vqsr.p.vcf.gz
# The extension `.vcf.gz` is automatically recognized.
```

1D query
```
pairix samples/SRR1171591.variants.snp.vqsr.p.vcf.gz chr10:1-4000000
chr10	3463966	.	C	T	51.74	PASS	AC=2;AF=1.00;AN=2;DB;DP=2;Dels=0.00;FS=0.000;HaplotypeScore=0.0000;MLEAC=2;MLEAF=1.00;MQ=50.00;MQ0=0;POSITIVE_TRAIN_SITE;QD=25.87;VQSLOD=7.88;culprit=FS	GT:AD:DP:GQ:PL	1/1:0,2:2:6:79,6,0
chr10	3978708	rs29320259	T	C	1916.77	PASS	AC=2;AF=1.00;AN=2;BaseQRankSum=1.016;DB;DP=67;Dels=0.00;FS=0.000;HaplotypeScore=629.1968;MLEAC=2;MLEAF=1.00;MQ=50.00;MQ0=0;MQRankSum=0.773;POSITIVE_TRAIN_SITE;QD=28.61;ReadPosRankSum=0.500;VQSLOD=3.29;culprit=FS	GT:AD:DP:GQ:PL	1/1:3,64:67:70:1945,70,0
chr10	3978709	.	G	A	1901.77	PASS	AC=2;AF=1.00;AN=2;BaseQRankSum=0.677;DB;DP=66;Dels=0.00;FS=0.000;HaplotypeScore=579.9049;MLEAC=2;MLEAF=1.00;MQ=50.00;MQ0=0;MQRankSum=0.308;POSITIVE_TRAIN_SITE;QD=28.81;ReadPosRankSum=0.585;VQSLOD=3.24;culprit=FS	GT:AD:DP:GQ:PL	1/1:3,63:66:73:1930,73,0
```

<br>

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

<br>

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
for x in it:
   print(x)

# 2D query usage example 2 with `querys2D(querystr)`
tb=pypairix.open("textfile.gz")
querystr='{}:{}-{}|{}:{}-{}'.format(chrom, start, end, chrom2, start2, end2)
it = tb.querys2D(querystr)
for x in it:
   print(x)

# 2D query usage example with wild card
tb=pypairix.open("textfile.gz")
querystr='{}:{}-{}|*'.format(chrom, start, end)
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

# get the column index (0-based)
tb=pypairix.open("textfile.gz")
print( tb.get_chr1_col() )
print( tb.get_chr2_col() )
print( tb.get_startpos1_col() )
print( tb.get_startpos2_col() )
print( tb.get_endpos1_col() )
print( tb.get_endpos2_col() )

# check if key exists (key is a chromosome pair for a 2D-indexed file, or a chromosome for a 1D-indexed file)
tb=pypairix.open("textfile.gz")
print( tb.exists("chr1|chr2") )  # 1 if exists, 0 if not.
```

<br>

## Rpairix
* Rpairix is an R package for reading pairix-indexed pairs files. It has its own repo: https://github.com/4dn-dcic/Rpairix

<br>

## Utils
### bam2pairs
* This script converts a bam file to a 4dn style pairs file, sorted and indexed.
* See [util/bam2pairs/README.md](util/bam2pairs/README.md) for more details.

### process_merged_nodup.sh
* This script sorts, bgzips and indexes a newer version of `merged_nodups.txt` file with strand1 as the first column.
```
Usage: process_merged_nodup.sh <merged_nodups.txt>
```

### process_old_merged_nodup.sh
* This script sorts, bgzips and indexes an old version of `merged_nodups.txt` file with readID as the first column.
```
Usage: process_old_merged_nodup.sh <merged_nodups.txt>
```

### merged_nodup2pairs.pl
* This script converts Juicer's `merged_nodups.txt` format to 4dn-style pairs format. It requires pairix and bgzip binaries in PATH.
```
Usage: merged_nodup2pairs.pl <input_merged_nodups.txt> <output_prefix>
```
* An example output file (bgzipped and indexed) looks as below.
```
## pairs format v1.0
#sorted: chr1-chr2-pos1-pos2
#shape: upper triangle
#columns: readID chr1 pos1 chr2 pos2 strand1 strand2 frag1 frag2
SRR1658650.8850202.2/2	1	16944943	1	151864549	-	+	45178	333257
SRR1658650.8794979.1/1	1	21969282	1	50573348	-	-	59146	140641
SRR1658650.6209714.1/1	1	31761397	1	32681095	-	+	88139	90865
SRR1658650.6348458.2/2	1	40697468	1	40698014	+	-	113763	113763
SRR1658650.12316544.1/1	1	41607001	1	41608253	+	+	116392	116398
```


### old_merged_nodup2pairs.pl
* This script converts Juicer's old `merged_nodups.txt` format to 4dn-style pairs format. It requires pairix and bgzip binaries in PATH.
```
Usage: old_merged_nodup2pairs.pl <input_merged_nodups.txt> <output_prefix>
```
* An example output file (bgzipped and indexed) looks as below.
```
## pairs format v1.0
#sorted: chr1-chr2-pos1-pos2
#shape: upper triangle
#columns: readID chr1 pos1 chr2 pos2 strand1 strand2 frag1 frag2
SRR1658650.8850202.2/2	1	16944943	1	151864549	-	+	45178	333257
SRR1658650.8794979.1/1	1	21969282	1	50573348	-	-	59146	140641
SRR1658650.6209714.1/1	1	31761397	1	32681095	-	+	88139	90865
SRR1658650.6348458.2/2	1	40697468	1	40698014	+	-	113763	113763
SRR1658650.12316544.1/1	1	41607001	1	41608253	+	+	116392	116398
```

### Pairs_merger
Pairs_merger is a tool that merges indexed pairs files that are already sorted and creates a sorted output pairs file. Pairs_merger uses a k-way merge sort algorithm starting with k file streams. Specifically, it loops over a merged iterator composed of a dynamically sorted array of k interators. It does not require additional memory nor produces temporary files.

#### Usage for pairs_merger
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

#### Usage Examples for pairs_merger
```
bin/pairs_merger samples/merged_nodups.space.chrblock_sorted.subsample2.txt.gz samples/merged_nodups.space.chrblock_sorted.subsample3.txt.gz | bin/bgzip -c > out.gz
bin/pairix -f -p merged_nodups out.gz
# The above command is equivalent to : bin/pairix -f -s2 -d6 -b3 -e3 -u7 -T out.gz
```


### Streamer_1d
Streamer_1d is a tool that converts a 2d-sorted pairs file to a 1d-sorted stream (sorted by chr1-chr2-pos1-pos2  ->  sorted by chr1-pos1). This tool uses a k-way merge sort on k file pointers on the same input file, operates linearly without producing any temporary files. Currently, the speed is actually slower than unix sort (not recommended).

#### Usage for streamer_1d
```
streamer_1d <in.2d.gz> > out.1d.pairs
streamer_1d <in.2d.gz> | bgzip -c > out.1d.pairs.gz
```

#### Usage Examples for streamer_1d
```
bin/streamer_1d samples/merged_nodups.space.chrblock_sorted.subsample2.txt.gz | bgzip -c > out.1d.pairs.gz
```

#### FAQ for streamer_1d
##### The tool creates many file pointers for the input file, which is equivalent to opening many files simultaneously. Your OS may have a limit on the number of files that can be open at a time. For example, for Mac El Captain and Sierra, it is by default set to 256. This is usually enough, but in case the number of chromosomes in your pairs file happen to be larger than or close to this limit, the tool may produce an error message saying file limit is exceeded. You can increase this limit outside the program. For example, for Mac El Captain and Sierra, the following command raises the limit to 2000.
```
# view the limits
uimit -a

# raise the limit to 2000
ulimit -n 2000
```

<br>

## Note
* Currently 2D indexing supports only 2D query (one of the mates can be a wildcard *) and 1D indexing supports only 1D query. Ideally, it will be extended to support 1D query for 2D indexed files. (future plan)
* Note that if the chromosome pair block are ordered in a way that the first coordinate is always smaller than the second ('upper-triangle'), a lower-triangle query will return an empty result. For example, if there is a block with chr1='6' and chr2='X', but not with chr1='X' and chr2='6', then the query for X|6 will not return any result. The search is not symmetric.
* Tabix and pairix indices are not cross-compatible.

<br>

## Version history

### 0.1.3
* added build_index methods. Now you can build index files (.px2) using command line, R, or Python
    + R: px_build_index(<filename>)
    + Python: pypairix.build_index(<filename>)
* tests updated
* pairs_format_specification updated

### 0.1.2
* Now custom column set overrides file extension recognition
* bam2pairs now records 5end of a read for position (instead of leftmost)
* Version number in the binaries fixed.

### 0.1.1
* Now all source files are in `src/`.
* `pypairix`: function `exists` is added
* `pairix`: indexing presets (-p option) now includes `pairs`, `merged_nodups`, `old_merged_nodups`. It also automatically recognizes extension `.pairs.gz`.
* merged_nodups.tab examples are now deprecated (since the original space-delimited files can be recognized as well)
* `pairs_merger`: memory error fixed
* updated tests
