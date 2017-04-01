# bam2pairs

This script converts a paired-end bam file to a pairs file.


## Installation
```
git clone https://github.com/4dn-dcic/bam2pairs
## add bam2pairs to PATH.
```

## Requirements
* samtools (https://github.com/samtools/samtools)
* bgzip (https://github.com/hms-dbmi/pairix or https://github.com/samtools/tabix)
* pairix (https://github.com/hms-dbmi/pairix)


## Input
The input bam file is paired-end and it doesn't have to be sorted or indexed.


## Output
The output file contains all of the mapped reads in the input bam file. Both of the mates must be mapped. No additional filtering is performed. Every line of the bam file that meets the criterion of an 'upper triangle' (mate1 < mate2) is added nonredundantly to the output pairs file.
The output file is an upper-triangular, chromosome-pair-block-sorted, pairs file, with the following columns.
```
readID chr1 pos1 chr2 pos2 strand1 strand2
```
It is bgzipped and indexed by pairix (https://github.com/hms-dbmi/pairix) on the chromosome pairs.


## Example Output
The first few lines of an example output file looks as below:
```
## pairs format v1.0
#sorted: chr1-chr2-pos1-pos2
#shape: upper triangle
#columns: readID chr1 pos1 chr2 pos2 strand1 strand2
#command: bam2pairs /d/bam/_1_out.sorted.bam /d/pairs/_1_out.sorted
SRR1658581.31870055     chr1    15398   chr1    53692634        -       +
SRR1658581.24590805     chr1    19593   chr1    11784792        +       +
SRR1658581.14238118     chr1    24349   chr1    67077   -       -
SRR1658581.5571806      chr1    56190   chr1    56543   +       -
SRR1658581.49762205     chr1    106813  chr1    252815  -       +
SRR1658581.25376859     chr1    114423  chr1    143957748       -       -
SRR1658581.30418686     chr1    134451  chr1    212858250       -       +
```
