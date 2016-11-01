# tabix
This modification can do double-chromosome indexing/query as an option.
The text file must be first sorted by two chromosome columns and then by the first position column. The file must be compressed using bgzip.

## installation
```
git clone https://github.com/SooLee/tabix.git
cd tabix
make
# add the current path to PATH
```


## Usage 
```
# compression
bgzip textfile

# indexing
tabix -s<chr1_column> [-d<chr2_column>] -b<pos1_start_column> -e<pos1_end_column> [-u<pos2_start_column> -v<pos2_end_column>] textfile.gz    # u, v is required for full 2d query.

# querying
tabix textfile.gz region1 [region2 [...]]  ## region is in the following format.
tabix textfile.gz '<chr1>[|<chr2>]:<start1>-<end1>[|<start2>-<end2>]'    # make sure to quote, so '|' is not interpreted as a pipe.
```

## Note
* Currently 2D indexing supports only 2D query and 1D indexing supports only 1D query. Ideally, it will be extended to support 1D query for 2D indexed files. (future plan)
* The index produced by this modified tabix is not compatible with the original tabix index. They are based on different structures.
* Note that if the chromosome pair block are ordered in a way that the first coordinate is always smaller than the second ('upper-triangle'), a lower-triangle query will return an empty result. For example, if there is a block with chr1='6' and chr2='X', but not with chr1='X' and chr2='6', then the query for X|6 will not return any result. The search is not symmetric.


## Example
### Preparing a double-chromosome-block sorted text file 
(column 2 and 6 are chromosomes (chr1 and chr2), column 3 is position of the first coordinate (pos1)).
```
cut -d' ' -f1-8 /n/data1/hms/dbmi/park/sl325/juicer/SRR1658832/aligned.20160803/merged_nodups.txt | sort -t' ' -k2,2 -k6,6 -k3,3g - > merged_nodup.chrblock_sorted.txt
sed 's/ /\t/g' merged_nodup.chrblock_sorted.txt > merged_nodup.tab.chrblock_sorted.txt
head merged_nodup.tab.chrblock_sorted.txt

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
bgzip merged_nodup.tab.chrblock_sorted.txt
```

### 2D indexing & query on the above file
2D indexing with tabix on chromosome pair (-s2 -d6) and the position of the first chromosome (b3). For full 2D query, also add -u7 and -v7, the start and end positions of the second coordinate. They are not used for indexing per se, but the column index is stored as part of the index, which allows full 2D query through individual comparisons.
```
./tabix -f -s2 -d6 -b3 -e3 -u7 -v7 merged_nodup.tab.chrblock_sorted.txt.gz
```
semi 2D query (chr10:1-1000000 x chr20)
```
./tabix merged_nodup.tab.chrblock_sorted.txt.gz '10|20:1-1000000'
0	10	624779	1361	0	20	40941397	97868
16	10	948577	2120	16	20	59816485	148396
```
full 2D query (chr10:1-1000000 x chr20:50000000-60000000)
```
./tabix merged_nodup.tab.chrblock_sorted.txt.gz '10|20:1-1000000|50000000-60000000'
16	10	948577	2120	16	20	59816485	148396
```
full 2D multi-query (chr1:1-10000000 x chr20:50000000-60000000 AND 3:5000000-9000000 x X:70000000-90000000)
```
./tabix merged_nodup.tab.chrblock_sorted.txt.gz '1|20:1-10000000|50000000-60000000' '3|X:5000000-9000000|70000000-90000000'
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


### 1D indexing on a regular vcf file, bgzipped.
1D indexing
```
./tabix -s1 -b2 -e2 -f SRR1171591.variants.snp.vqsr.p.vcf.gz
```
1D query
```
./tabix SRR1171591.variants.snp.vqsr.p.vcf.gz chr10:1-4000000
chr10	3463966	.	C	T	51.74	PASS	AC=2;AF=1.00;AN=2;DB;DP=2;Dels=0.00;FS=0.000;HaplotypeScore=0.0000;MLEAC=2;MLEAF=1.00;MQ=50.00;MQ0=0;POSITIVE_TRAIN_SITE;QD=25.87;VQSLOD=7.88;culprit=FS	GT:AD:DP:GQ:PL	1/1:0,2:2:6:79,6,0
chr10	3978708	rs29320259	T	C	1916.77	PASS	AC=2;AF=1.00;AN=2;BaseQRankSum=1.016;DB;DP=67;Dels=0.00;FS=0.000;HaplotypeScore=629.1968;MLEAC=2;MLEAF=1.00;MQ=50.00;MQ0=0;MQRankSum=0.773;POSITIVE_TRAIN_SITE;QD=28.61;ReadPosRankSum=0.500;VQSLOD=3.29;culprit=FS	GT:AD:DP:GQ:PL	1/1:3,64:67:70:1945,70,0
chr10	3978709	.	G	A	1901.77	PASS	AC=2;AF=1.00;AN=2;BaseQRankSum=0.677;DB;DP=66;Dels=0.00;FS=0.000;HaplotypeScore=579.9049;MLEAC=2;MLEAF=1.00;MQ=50.00;MQ0=0;MQRankSum=0.308;POSITIVE_TRAIN_SITE;QD=28.81;ReadPosRankSum=0.585;VQSLOD=3.24;culprit=FS	GT:AD:DP:GQ:PL	1/1:3,63:66:73:1930,73,0
```


