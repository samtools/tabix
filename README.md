# bam2pairs

This script converts a paired-end bam file to a pairs file.


## Installation
```
git clone https://github.com/4dn-dcic/bam2pairs
## add bam2pairs to PATH.
```


## Input
The input bam file is paired-end and it doesn't have to be sorted or indexed.


## Output
The output file contains all of the mapped reads in the input bam file. Both of the mates must be mapped. No additional filtering is performed.
The output file is a Uc (upper-triangle, chromosome-block-sorted) pairs file, with the following columns.
```
readID chr1 pos1 chr2 pos2 strand1 strand2
```
It is bgzipped and indexed by pairix (https://github.com/hms-dbmi/pairix) on the chromosome pairs.

